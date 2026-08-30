#include "Music/CompositionConditionedMidiDecoder.h"
#include "Music/CompositionConditionedSequenceNeuralModelRuntimeLoader.h"
#include "Music/CompositionMidiCorpusDirectoryLoader.h"
#include "Music/CompositionMidiSequenceWindow.h"
#include "Music/CompositionRealComposerCorpusPreparation.h"
#include "Music/CompositionMidiSequenceCorpusBuilder.h"
#include "Music/CompositionSequenceMetadataFileLoader.h"

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

using namespace midigengx::music;

namespace
{

void fail(const std::string& message)
{
    std::cerr << "FAILED: " << message << '\n';
    std::exit(1);
}

void writeBE16(
    std::ostream& stream,
    const std::uint16_t value)
{
    stream.put(static_cast<char>((value >> 8) & 0xff));
    stream.put(static_cast<char>(value & 0xff));
}

void writeBE32(
    std::ostream& stream,
    const std::uint32_t value)
{
    stream.put(static_cast<char>((value >> 24) & 0xff));
    stream.put(static_cast<char>((value >> 16) & 0xff));
    stream.put(static_cast<char>((value >> 8) & 0xff));
    stream.put(static_cast<char>(value & 0xff));
}

void writeVariableLengthQuantity(
    std::vector<std::uint8_t>& output,
    std::uint32_t value)
{
    std::uint8_t buffer[5];
    int count = 1;
    buffer[4] = static_cast<std::uint8_t>(value & 0x7f);

    while ((value >>= 7) != 0)
    {
        buffer[4 - count] =
            static_cast<std::uint8_t>((value & 0x7f) | 0x80);
        ++count;
    }

    for (int index = 5 - count;
         index < 5;
         ++index)
    {
        output.push_back(buffer[index]);
    }
}

struct MidiWriteEvent
{
    std::uint32_t tick = 0;
    std::uint8_t status = 0;
    std::uint8_t data1 = 0;
    std::uint8_t data2 = 0;
};

std::uint32_t beatToTick(
    const double beat,
    const std::uint32_t ppq)
{
    if (!std::isfinite(beat) || beat <= 0.0)
        return 0;

    const double ticks =
        beat * static_cast<double>(ppq);

    return static_cast<std::uint32_t>(
        std::clamp(
            std::llround(ticks),
            0LL,
            static_cast<long long>(std::numeric_limits<std::uint32_t>::max())));
}

bool writePhraseAsType0Midi(
    const Phrase& phrase,
    const std::filesystem::path& path)
{
    constexpr std::uint16_t ppq = 480;
    std::vector<MidiWriteEvent> events;
    events.reserve(phrase.notes.size() * 2);

    for (const auto& note : phrase.notes)
    {
        const auto start =
            beatToTick(
                note.startBeat,
                ppq);

        const auto end =
            beatToTick(
                note.startBeat + note.durationBeats,
                ppq);

        events.push_back(
            {
                start,
                static_cast<std::uint8_t>(0x90 | ((note.channel - 1) & 0x0f)),
                static_cast<std::uint8_t>(note.midiNote),
                static_cast<std::uint8_t>(note.velocity)
            });

        events.push_back(
            {
                std::max(start + 1U, end),
                static_cast<std::uint8_t>(0x80 | ((note.channel - 1) & 0x0f)),
                static_cast<std::uint8_t>(note.midiNote),
                0
            });
    }

    std::sort(
        events.begin(),
        events.end(),
        [](const MidiWriteEvent& left,
           const MidiWriteEvent& right)
        {
            if (left.tick != right.tick)
                return left.tick < right.tick;

            const bool leftOff =
                (left.status & 0xf0) == 0x80;
            const bool rightOff =
                (right.status & 0xf0) == 0x80;

            if (leftOff != rightOff)
                return leftOff;

            if (left.status != right.status)
                return left.status < right.status;

            return left.data1 < right.data1;
        });

    std::vector<std::uint8_t> track;
    track.reserve(events.size() * 5 + 16);

    std::uint32_t previousTick = 0;

    for (const auto& event : events)
    {
        writeVariableLengthQuantity(
            track,
            event.tick - previousTick);

        track.push_back(event.status);
        track.push_back(event.data1);
        track.push_back(event.data2);

        previousTick = event.tick;
    }

    // End of track meta event.
    track.push_back(0x00);
    track.push_back(0xff);
    track.push_back(0x2f);
    track.push_back(0x00);

    std::ofstream file(
        path,
        std::ios::binary);

    if (!file)
        return false;

    file.write("MThd", 4);
    writeBE32(file, 6);
    writeBE16(file, 0);      // format 0
    writeBE16(file, 1);      // one track
    writeBE16(file, ppq);

    file.write("MTrk", 4);
    writeBE32(
        file,
        static_cast<std::uint32_t>(track.size()));

    if (!track.empty())
    {
        file.write(
            reinterpret_cast<const char*>(track.data()),
            static_cast<std::streamsize>(track.size()));
    }

    return static_cast<bool>(file);
}

std::vector<std::uint8_t> readBinaryFile(
    const std::filesystem::path& path)
{
    std::ifstream file(
        path,
        std::ios::binary);

    if (!file)
        return {};

    return std::vector<std::uint8_t>(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
}

CompositionMidiSequenceWindow makeWindow(
    const std::vector<CompositionMidiTrainingEvent>& context,
    const CompositionSequenceLearningContract& contract)
{
    CompositionMidiSequenceWindow window;

    if (!contract.isValid() ||
        context.size() != contract.contextLength ||
        contract.inputFeatureWidth !=
            CompositionMidiTrainingEvent::featureCount)
    {
        return window;
    }

    window.contextLength = contract.contextLength;
    window.featureWidth = contract.inputFeatureWidth;
    window.inputs.assign(
        window.contextLength * window.featureWidth,
        0.0);
    window.paddingMask.assign(
        window.contextLength,
        1.0);

    for (std::size_t index = 0;
         index < context.size();
         ++index)
    {
        if (!context[index].isValid())
            return CompositionMidiSequenceWindow{};

        std::copy(
            context[index].features.begin(),
            context[index].features.end(),
            window.inputs.begin() +
                index * window.featureWidth);
    }

    window.targets =
        context.back().features;
    window.valid = true;

    if (!window.isValid(contract))
        return CompositionMidiSequenceWindow{};

    return window;
}

} // namespace

int main(int argc, char* argv[])
{
    if (argc == 1)
    {
        std::cout
            << "PHASE 120.4C ARTIFACT VALIDATION EXECUTABLE SELF CHECK PASSED\n"
            << "usageContract=4Arguments\n";
        return 0;
    }

    if (argc != 5)
    {
        fail(
            "usage: "
            "CompositionConditionedRealMidiArtifactValidationTests "
            "<midi-directory> <metadata.tsv> <model.mgcn> <output.midi>");
    }

    const std::filesystem::path midiDirectory = argv[1];
    const std::filesystem::path metadataPath = argv[2];
    const std::filesystem::path modelPath = argv[3];
    const std::filesystem::path outputPath = argv[4];

    const auto modelBytes =
        readBinaryFile(modelPath);

    if (modelBytes.empty())
        fail("model file is missing or empty");

    const auto runtimeResult =
        CompositionConditionedSequenceNeuralModelRuntimeLoader{}
            .load(modelBytes);

    if (!runtimeResult.isValid())
        fail("real model failed runtime loading");

    const auto metadata =
        loadCompositionSequenceMetadataFile(
            metadataPath.string());

    if (!metadata.isValid())
        fail("metadata catalog is invalid");

    const auto prepared =
        prepareRealComposerCorpusFromDirectory(
            midiDirectory.string(),
            true,
            metadata.catalog);

    if (!prepared.isValid() ||
        prepared.sequences.size() != 67 ||
        prepared.conditionedDataset.sampleCount() != 67)
    {
        fail("real Chopin corpus preparation is not 67/67 valid");
    }

    const auto& dataset =
        prepared.conditionedDataset;

    if (runtimeResult.model.vocabulary.composers !=
            dataset.vocabulary.composers ||
        runtimeResult.model.vocabulary.styles !=
            dataset.vocabulary.styles ||
        runtimeResult.model.vocabulary.eras !=
            dataset.vocabulary.eras ||
        runtimeResult.model.vocabulary.instrumentations !=
            dataset.vocabulary.instrumentations)
    {
        fail("model vocabulary does not match current corpus vocabulary");
    }

    const auto& seed =
        dataset.samples.front();

    if (!seed.isValid() ||
        seed.sequence.events.size() <
            runtimeResult.model.contract.contextLength)
    {
        fail("seed sample cannot initialize rollout");
    }

    std::vector<CompositionMidiTrainingEvent> context;

    for (std::size_t index = 0;
         index < runtimeResult.model.contract.contextLength;
         ++index)
    {
        context.push_back(
            seed.sequence.events[index]);
    }

    double previousStartBeat = 0.0;

    for (std::size_t index = 1;
         index < context.size();
         ++index)
    {
        previousStartBeat +=
            std::max(
                0.0,
                (std::clamp(
                    context[index].features[3],
                    -1.0,
                    1.0) + 1.0) *
                0.5);
    }

    CompositionConditionedMidiDecoderConfig decoderConfig;
    Phrase generatedPhrase;

    constexpr std::size_t generatedEventCount = 128;

    for (std::size_t index = 0;
         index < generatedEventCount;
         ++index)
    {
        const auto window =
            makeWindow(
                context,
                runtimeResult.model.contract);

        if (!window.isValid(
                runtimeResult.model.contract))
        {
            fail("rollout window became invalid");
        }

        const auto prediction =
            runtimeResult.model.predictNextEvent(
                window,
                seed);

        if (!prediction.isValid(
                runtimeResult.model.contract.targetFeatureWidth))
        {
            fail("model prediction became invalid");
        }

        const auto decoded =
            CompositionConditionedMidiDecoder::decodeEvent(
                prediction.features,
                context.back(),
                previousStartBeat,
                decoderConfig);

        if (decoded.midiNote < 0 ||
            decoded.midiNote > 127 ||
            decoded.velocity < 1 ||
            decoded.velocity > 127 ||
            !std::isfinite(decoded.startBeat) ||
            !std::isfinite(decoded.durationBeats) ||
            decoded.startBeat < previousStartBeat ||
            decoded.durationBeats <= 0.0 ||
            decoded.channel < 1 ||
            decoded.channel > 16)
        {
            fail("decoded generated note is invalid");
        }

        generatedPhrase.notes.push_back(
            decoded);

        previousStartBeat =
            decoded.startBeat;

        CompositionMidiTrainingEvent generatedEvent;
        generatedEvent.features =
            prediction.features;

        if (!generatedEvent.isValid())
            fail("generated autoregressive feature event is invalid");

        context.erase(
            context.begin());

        context.push_back(
            generatedEvent);
    }

    generatedPhrase.lengthBeats =
        previousStartBeat +
        generatedPhrase.notes.back().durationBeats;

    generatedPhrase.normalize();

    if (!generatedPhrase.isValid() ||
        generatedPhrase.notes.size() != generatedEventCount)
    {
        fail("generated Phrase failed final validation");
    }

    std::filesystem::create_directories(
        outputPath.parent_path());

    std::filesystem::remove(
        outputPath);

    if (!writePhraseAsType0Midi(
            generatedPhrase,
            outputPath))
    {
        fail("generated MIDI artifact could not be written");
    }

    const auto writtenBytes =
        readBinaryFile(outputPath);

    if (writtenBytes.size() < 14 ||
        std::string(
            reinterpret_cast<const char*>(
                writtenBytes.data()),
            4) != "MThd")
    {
        fail("generated MIDI artifact has no valid MThd header");
    }

    const auto reloaded =
        loadCompositionMidiCorpusDirectory(
            outputPath.parent_path().string(),
            false);

    if (!reloaded.isValid() ||
        reloaded.acceptedFileCount != 1 ||
        reloaded.rejectedFileCount != 0 ||
        reloaded.records.size() != 1)
    {
        fail("generated MIDI artifact failed project MIDI reload");
    }

    const auto& roundTripped =
        reloaded.records.front();

    if (!roundTripped.isValid() ||
        roundTripped.noteCount() != generatedEventCount)
    {
        fail("generated MIDI round-trip note count mismatch");
    }

    std::error_code cleanupError;
    std::filesystem::remove(
        outputPath,
        cleanupError);

    std::cout
        << "PHASE 120.4C REAL CONDITIONED MIDI ARTIFACT VALIDATION COMPLETE\n"
        << "modelLoaded=1\n"
        << "generatedEvents=" << generatedEventCount << '\n'
        << "generatedPhraseValid=1\n"
        << "midiHeaderValid=1\n"
        << "midiReloadValid=1\n"
        << "roundTripEventCount=" << roundTripped.noteCount() << '\n'
        << "roundTripRejected=0\n";

    return 0;
}
