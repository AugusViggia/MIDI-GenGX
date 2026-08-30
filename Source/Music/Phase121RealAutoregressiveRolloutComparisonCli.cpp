#include "Music/CompositionConditionedMidiDecoder.h"
#include "Music/CompositionConditionedSequenceNeuralModelRuntimeLoader.h"
#include "Music/CompositionMidiCorpusDirectoryLoader.h"
#include "Music/CompositionMidiSequenceWindow.h"
#include "Music/CompositionRealComposerCorpusPreparation.h"
#include "Music/CompositionSequenceMetadataFileLoader.h"

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <iterator>
#include <string>
#include <vector>

using namespace midigengx::music;

namespace
{

struct MidiWriteEvent
{
    std::uint32_t tick = 0;
    std::uint8_t status = 0;
    std::uint8_t data1 = 0;
    std::uint8_t data2 = 0;
};

void writeBE16(std::ostream& s, std::uint16_t v)
{
    s.put(static_cast<char>((v >> 8) & 0xff));
    s.put(static_cast<char>(v & 0xff));
}

void writeBE32(std::ostream& s, std::uint32_t v)
{
    s.put(static_cast<char>((v >> 24) & 0xff));
    s.put(static_cast<char>((v >> 16) & 0xff));
    s.put(static_cast<char>((v >> 8) & 0xff));
    s.put(static_cast<char>(v & 0xff));
}

void writeVLQ(
    std::vector<std::uint8_t>& out,
    std::uint32_t value)
{
    std::uint8_t buffer[5];
    int count = 1;
    buffer[4] =
        static_cast<std::uint8_t>(value & 0x7f);

    while ((value >>= 7) != 0)
    {
        buffer[4 - count] =
            static_cast<std::uint8_t>(
                (value & 0x7f) | 0x80);
        ++count;
    }

    for (int index = 5 - count;
         index < 5;
         ++index)
        out.push_back(buffer[index]);
}

std::uint32_t beatToTick(double beat)
{
    constexpr std::uint32_t ppq = 480;

    if (!std::isfinite(beat) || beat <= 0.0)
        return 0;

    return static_cast<std::uint32_t>(
        std::clamp(
            std::llround(
                beat * static_cast<double>(ppq)),
            0LL,
            static_cast<long long>(
                std::numeric_limits<std::uint32_t>::max())));
}

bool writeMidi(
    const Phrase& phrase,
    const std::filesystem::path& path)
{
    constexpr std::uint16_t ppq = 480;

    std::vector<MidiWriteEvent> events;
    events.reserve(
        phrase.notes.size() * 2);

    for (const auto& note : phrase.notes)
    {
        const auto start =
            beatToTick(note.startBeat);

        const auto end =
            beatToTick(
                note.startBeat +
                note.durationBeats);

        events.push_back(
        {
            start,
            static_cast<std::uint8_t>(
                0x90 |
                ((note.channel - 1) & 0x0f)),
            static_cast<std::uint8_t>(
                note.midiNote),
            static_cast<std::uint8_t>(
                note.velocity)
        });

        events.push_back(
        {
            std::max(start + 1U, end),
            static_cast<std::uint8_t>(
                0x80 |
                ((note.channel - 1) & 0x0f)),
            static_cast<std::uint8_t>(
                note.midiNote),
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

            return left.data1 < right.data1;
        });

    std::vector<std::uint8_t> track;
    track.reserve(
        events.size() * 5 + 4);

    std::uint32_t previousTick = 0;

    for (const auto& event : events)
    {
        writeVLQ(
            track,
            event.tick - previousTick);

        track.push_back(event.status);
        track.push_back(event.data1);
        track.push_back(event.data2);

        previousTick =
            event.tick;
    }

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
    writeBE16(file, 0);
    writeBE16(file, 1);
    writeBE16(file, ppq);

    file.write("MTrk", 4);
    writeBE32(
        file,
        static_cast<std::uint32_t>(
            track.size()));

    if (!track.empty())
    {
        file.write(
            reinterpret_cast<const char*>(
                track.data()),
            static_cast<std::streamsize>(
                track.size()));
    }

    return static_cast<bool>(file);
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

    window.contextLength =
        contract.contextLength;

    window.featureWidth =
        contract.inputFeatureWidth;

    window.inputs.assign(
        window.contextLength *
            window.featureWidth,
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

struct RolloutStats
{
    std::size_t generatedEvents = 0;
    std::size_t uniqueNotes = 0;
    double totalDurationBeats = 0.0;
    double averageVelocity = 0.0;
    double averageDurationBeats = 0.0;
};

bool runRollout(
    const CompositionConditionedSequenceNeuralModel& model,
    const CompositionConditionedTrainingSample& seed,
    const std::filesystem::path& output,
    RolloutStats& stats)
{
    if (!model.isValid() ||
        !seed.isValid() ||
        seed.sequence.events.size() <
            model.contract.contextLength)
    {
        return false;
    }

    std::vector<CompositionMidiTrainingEvent> context;

    for (std::size_t index = 0;
         index < model.contract.contextLength;
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

    Phrase generated;

    constexpr std::size_t generatedEventCount = 256;

    std::int64_t velocitySum = 0;
    double durationSum = 0.0;
    std::vector<bool> seen(128, false);

    for (std::size_t index = 0;
         index < generatedEventCount;
         ++index)
    {
        const auto window =
            makeWindow(
                context,
                model.contract);

        if (!window.isValid(model.contract))
            return false;

        const auto prediction =
            model.predictNextEvent(
                window,
                seed);

        if (!prediction.isValid(
                model.contract.targetFeatureWidth))
        {
            return false;
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
            return false;
        }

        generated.notes.push_back(decoded);

        seen[
            static_cast<std::size_t>(
                decoded.midiNote)] = true;

        velocitySum +=
            decoded.velocity;

        durationSum +=
            decoded.durationBeats;

        previousStartBeat =
            decoded.startBeat;

        CompositionMidiTrainingEvent generatedEvent;
        generatedEvent.features =
            prediction.features;

        constexpr double kPpq = 480.0;

        const auto previousEvent = context.back();

        const auto previousPitch =
            static_cast<int>(
                std::clamp(
                    std::lround(
                        previousEvent.features[0] *
                        127.0),
                    0L,
                    127L));

        const auto priorStartBeat =
            previousStartBeat;

        const auto startBeat =
            decoded.startBeat;

        const auto deltaBeats =
            std::max(
                0.0,
                startBeat -
                priorStartBeat);

        // Rebuild the dynamic event fields from the actual decoded MIDI
        // event so the next model step receives a self-consistent context.
        // Structural/harmonic guidance predicted by the model is preserved.
        generatedEvent.features[0] =
            std::clamp(
                static_cast<double>(
                    decoded.midiNote) /
                127.0,
                0.0,
                1.0);

        generatedEvent.features[1] =
            std::clamp(
                static_cast<double>(
                    decoded.velocity) /
                127.0,
                0.0,
                1.0);

        generatedEvent.features[2] =
            std::clamp(
                decoded.durationBeats / 8.0,
                0.0,
                1.0);

        generatedEvent.features[3] =
            std::clamp(
                (deltaBeats * kPpq) /
                (kPpq * 4.0),
                0.0,
                1.0);

        generatedEvent.features[4] =
            std::clamp(
                std::fmod(
                    std::max(0.0, startBeat),
                    4.0) /
                4.0,
                0.0,
                1.0);

        generatedEvent.features[5] =
            std::clamp(
                std::fmod(
                    std::max(0.0, startBeat),
                    1.0),
                0.0,
                1.0);

        generatedEvent.features[6] =
            std::clamp(
                static_cast<double>(
                    decoded.midiNote % 12) /
                11.0,
                0.0,
                1.0);

        generatedEvent.features[7] =
            std::clamp(
                static_cast<double>(
                    decoded.midiNote / 12) /
                10.0,
                0.0,
                1.0);

        generatedEvent.features[8] =
            std::clamp(
                static_cast<double>(
                    decoded.midiNote -
                    previousPitch) /
                12.0,
                -1.0,
                1.0);

        generatedEvent.features[9] =
            std::clamp(
                decoded.durationBeats,
                0.0,
                1.0);

        generatedEvent.features[19] =
            std::clamp(
                static_cast<double>(
                    decoded.channel) /
                15.0,
                0.0,
                1.0);

        if (!generatedEvent.isValid())
            return false;

        context.erase(context.begin());
        context.push_back(generatedEvent);
    }

    generated.lengthBeats =
        previousStartBeat +
        generated.notes.back().durationBeats;

    generated.normalize();

    if (!generated.isValid())
        return false;

    if (!writeMidi(
            generated,
            output))
    {
        return false;
    }

    stats.generatedEvents =
        generated.notes.size();

    stats.uniqueNotes =
        static_cast<std::size_t>(
            std::count(
                seen.begin(),
                seen.end(),
                true));

    stats.totalDurationBeats =
        generated.lengthBeats;

    stats.averageVelocity =
        static_cast<double>(
            velocitySum) /
        static_cast<double>(
            generated.notes.size());

    stats.averageDurationBeats =
        durationSum /
        static_cast<double>(
            generated.notes.size());

    return true;
}

bool loadModel(
    const std::string& path,
    CompositionConditionedSequenceNeuralModel& model)
{
    std::ifstream file(
        path,
        std::ios::binary);

    if (!file)
        return false;

    const std::istreambuf_iterator<char> first(file);
    const std::istreambuf_iterator<char> last;

    const std::vector<std::uint8_t> bytes(
        first,
        last);

    if (bytes.empty())
        return false;

    const auto result =
        CompositionConditionedSequenceNeuralModelRuntimeLoader{}
            .load(bytes);

    if (!result.isValid())
        return false;

    model = result.model;
    return true;
}

}

int main(int argc, char* argv[])
{
    if (argc != 6)
    {
        std::cerr
            << "FAILED: usage: "
            << "Phase121RealAutoregressiveRolloutComparison "
            << "<midi-directory> <metadata.tsv> "
            << "<phase119.mgcn> <phase121.mgcn> "
            << "<output-directory>\n";
        return 1;
    }

    const std::string midiDirectory = argv[1];
    const std::string metadataPath = argv[2];
    const std::string phase119Path = argv[3];
    const std::string phase121Path = argv[4];
    const std::filesystem::path outputDirectory = argv[5];

    const auto metadata =
        loadCompositionSequenceMetadataFile(
            metadataPath);

    if (!metadata.isValid())
    {
        std::cerr << "FAILED: metadata invalid\n";
        return 1;
    }

    const auto prepared =
        prepareRealComposerCorpusFromDirectory(
            midiDirectory,
            true,
            metadata.catalog);

    if (!prepared.isValid() ||
        prepared.sequences.size() != 67 ||
        prepared.conditionedDataset.sampleCount() != 67)
    {
        std::cerr
            << "FAILED: expected valid 67-sample corpus\n";
        return 1;
    }

    const auto& seed =
        prepared.conditionedDataset.samples.front();

    CompositionConditionedSequenceNeuralModel phase119;
    CompositionConditionedSequenceNeuralModel phase121;

    if (!loadModel(phase119Path, phase119) ||
        !loadModel(phase121Path, phase121))
    {
        std::cerr
            << "FAILED: model loading failed\n";
        return 1;
    }

    if (phase119.vocabulary.composers !=
            prepared.conditionedDataset.vocabulary.composers ||
        phase119.vocabulary.styles !=
            prepared.conditionedDataset.vocabulary.styles ||
        phase119.vocabulary.eras !=
            prepared.conditionedDataset.vocabulary.eras ||
        phase119.vocabulary.instrumentations !=
            prepared.conditionedDataset.vocabulary.instrumentations ||
        phase121.vocabulary.composers !=
            prepared.conditionedDataset.vocabulary.composers ||
        phase121.vocabulary.styles !=
            prepared.conditionedDataset.vocabulary.styles ||
        phase121.vocabulary.eras !=
            prepared.conditionedDataset.vocabulary.eras ||
        phase121.vocabulary.instrumentations !=
            prepared.conditionedDataset.vocabulary.instrumentations)
    {
        std::cerr
            << "FAILED: model vocabulary mismatch\n";
        return 1;
    }

    std::error_code error;
    std::filesystem::create_directories(
        outputDirectory,
        error);

    if (error)
    {
        std::cerr
            << "FAILED: cannot create output directory\n";
        return 1;
    }

    const auto phase119Output =
        outputDirectory /
        "phase119-autoregressive-256events.mid";

    const auto phase121Output =
        outputDirectory /
        "phase121-autoregressive-256events.mid";

    RolloutStats phase119Stats;
    RolloutStats phase121Stats;

    if (!runRollout(
            phase119,
            seed,
            phase119Output,
            phase119Stats))
    {
        std::cerr
            << "FAILED: Phase 119 rollout failed\n";
        return 1;
    }

    if (!runRollout(
            phase121,
            seed,
            phase121Output,
            phase121Stats))
    {
        std::cerr
            << "FAILED: Phase 121 rollout failed\n";
        return 1;
    }

    std::ofstream report(
        "phase121-real-autoregressive-rollout-comparison.txt");

    if (!report)
    {
        std::cerr
            << "FAILED: cannot create project-root report\n";
        return 1;
    }

    report
        << "PHASE 121 REAL AUTOREGRESSIVE ROLLOUT COMPARISON\n"
        << "generatedEventCount="
        << phase119Stats.generatedEvents
        << '\n'
        << "phase119.generatedEvents="
        << phase119Stats.generatedEvents
        << '\n'
        << "phase119.uniqueNotes="
        << phase119Stats.uniqueNotes
        << '\n'
        << "phase119.totalDurationBeats="
        << phase119Stats.totalDurationBeats
        << '\n'
        << "phase119.averageVelocity="
        << phase119Stats.averageVelocity
        << '\n'
        << "phase119.averageDurationBeats="
        << phase119Stats.averageDurationBeats
        << '\n'
        << "phase121.generatedEvents="
        << phase121Stats.generatedEvents
        << '\n'
        << "phase121.uniqueNotes="
        << phase121Stats.uniqueNotes
        << '\n'
        << "phase121.totalDurationBeats="
        << phase121Stats.totalDurationBeats
        << '\n'
        << "phase121.averageVelocity="
        << phase121Stats.averageVelocity
        << '\n'
        << "phase121.averageDurationBeats="
        << phase121Stats.averageDurationBeats
        << '\n'
        << "phase119Output="
        << phase119Output.string()
        << '\n'
        << "phase121Output="
        << phase121Output.string()
        << '\n'
        << "rolloutValid=1\n";

    report.flush();

    std::cout
        << "PHASE 121 REAL AUTOREGRESSIVE ROLLOUT COMPARISON COMPLETE\n"
        << "generatedEventCount="
        << phase119Stats.generatedEvents
        << '\n'
        << "phase119.uniqueNotes="
        << phase119Stats.uniqueNotes
        << '\n'
        << "phase121.uniqueNotes="
        << phase121Stats.uniqueNotes
        << '\n'
        << "phase119.totalDurationBeats="
        << phase119Stats.totalDurationBeats
        << '\n'
        << "phase121.totalDurationBeats="
        << phase121Stats.totalDurationBeats
        << '\n'
        << "rolloutValid=1\n";

    return 0;
}
