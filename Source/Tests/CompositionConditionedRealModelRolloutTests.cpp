#include "Music/CompositionConditionedMidiDecoder.h"
#include "Music/CompositionConditionedSequenceNeuralModelRuntimeLoader.h"
#include "Music/CompositionRealComposerCorpusPreparation.h"
#include "Music/CompositionSequenceMetadataFileLoader.h"
#include "Music/CompositionMidiSequenceWindow.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
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

std::vector<std::uint8_t> readBinaryFile(
    const std::string& path)
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

bool finiteBoundedVector(
    const std::vector<double>& values) noexcept
{
    if (values.empty())
        return false;

    for (const auto value : values)
    {
        if (!std::isfinite(value) ||
            value < -1.0 ||
            value > 1.0)
            return false;
    }

    return true;
}

CompositionMidiSequenceWindow
makeWindow(
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
        0.0);

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

        window.paddingMask[index] = 1.0;
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
            << "PHASE 120.4B ROLLOUT EXECUTABLE SELF CHECK PASSED\n"
            << "usageContract=3Arguments\n";
        return 0;
    }

    if (argc != 4)
    {
        fail(
            "usage: "
            "CompositionConditionedRealModelRolloutTests "
            "<midi-directory> <metadata.tsv> <model.mgcn>");
    }

    const std::string midiDirectory = argv[1];
    const std::string metadataPath = argv[2];
    const std::string modelPath = argv[3];

    const auto modelBytes =
        readBinaryFile(modelPath);

    if (modelBytes.empty())
        fail("model file is missing or empty");

    CompositionConditionedSequenceNeuralModelRuntimeLoader loader;

    const auto loaded =
        loader.load(modelBytes);

    if (!loaded.isValid())
        fail("real Phase 119 model failed runtime loading");

    const auto metadata =
        loadCompositionSequenceMetadataFile(
            metadataPath);

    if (!metadata.isValid())
        fail("metadata catalog is invalid");

    const auto prepared =
        prepareRealComposerCorpusFromDirectory(
            midiDirectory,
            true,
            metadata.catalog);

    if (!prepared.isValid())
        fail("real composer corpus preparation failed");

    if (prepared.inputSampleCount != 67 ||
        prepared.acceptedSampleCount != 67 ||
        prepared.rejectedSampleCount != 0 ||
        prepared.sequences.size() != 67 ||
        prepared.conditionedDataset.sampleCount() != 67)
    {
        fail("real Chopin corpus acceptance changed unexpectedly");
    }

    const auto& dataset =
        prepared.conditionedDataset;

    if (!dataset.isValid())
        fail("conditioned dataset is invalid");

    if (loaded.model.vocabulary.composers !=
            dataset.vocabulary.composers ||
        loaded.model.vocabulary.styles !=
            dataset.vocabulary.styles ||
        loaded.model.vocabulary.eras !=
            dataset.vocabulary.eras ||
        loaded.model.vocabulary.instrumentations !=
            dataset.vocabulary.instrumentations)
    {
        fail("real model vocabulary does not match current conditioned corpus");
    }

    if (dataset.samples.empty())
        fail("conditioned corpus has no samples");

    const auto& seed =
        dataset.samples.front();

    if (!seed.isValid())
        fail("seed conditioned sample is invalid");

    const auto contextLength =
        loaded.model.contract.contextLength;

    if (seed.sequence.events.size() <
        contextLength)
    {
        fail("seed sequence is shorter than model context length");
    }

    std::vector<CompositionMidiTrainingEvent> context;

    for (std::size_t index = 0;
         index < contextLength;
         ++index)
    {
        context.push_back(
            seed.sequence.events[index]);
    }

    CompositionConditionedMidiDecoderConfig decoderConfig;

    std::vector<NoteEvent> generatedNotes;
    generatedNotes.reserve(64);

    double previousStartBeat = 0.0;

    // Reconstruct the seed timeline from the same normalized delta feature
    // used by the training representation.
    for (std::size_t index = 1;
         index < context.size();
         ++index)
    {
        const auto& features =
            context[index].features;

        if (features.size() <= 3)
            fail("seed event has no delta feature");

        previousStartBeat +=
            std::max(
                0.0,
                (std::clamp(features[3], -1.0, 1.0) + 1.0) *
                0.5);
    }

    constexpr std::size_t rolloutEventCount = 64;

    for (std::size_t index = 0;
         index < rolloutEventCount;
         ++index)
    {
        const auto window =
            makeWindow(
                context,
                loaded.model.contract);

        if (!window.isValid(
                loaded.model.contract))
        {
            fail("autoregressive rollout window is invalid");
        }

        const auto prediction =
            loaded.model.predictNextEvent(
                window,
                seed);

        if (!prediction.isValid(
                loaded.model.contract.targetFeatureWidth) ||
            !finiteBoundedVector(
                prediction.features))
        {
            fail("real model produced an invalid prediction");
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
            decoded.startBeat < previousStartBeat ||
            !std::isfinite(decoded.durationBeats) ||
            decoded.durationBeats <= 0.0 ||
            decoded.channel < 1 ||
            decoded.channel > 16)
        {
            fail("decoded real-model event is invalid");
        }

        generatedNotes.push_back(
            decoded);

        previousStartBeat =
            decoded.startBeat;

        context.erase(
            context.begin());

        CompositionMidiTrainingEvent generatedEvent;
        generatedEvent.features =
            prediction.features;

        if (!generatedEvent.isValid())
            fail("generated model event features are invalid");

        context.push_back(
            generatedEvent);
    }

    Phrase generatedPhrase;
    generatedPhrase.notes =
        generatedNotes;
    generatedPhrase.lengthBeats =
        previousStartBeat +
        generatedNotes.back().durationBeats;

    generatedPhrase.normalize();

    if (!generatedPhrase.isValid())
        fail("real-model generated Phrase is invalid");

    std::cout
        << "PHASE 120.4B REAL CONDITIONED MODEL ROLLOUT COMPLETE\n"
        << "modelLoaded=1\n"
        << "corpusSequences=" << prepared.sequences.size() << '\n'
        << "conditionedSamples=" << dataset.sampleCount() << '\n'
        << "seedSampleId=" << seed.sequence.sampleId << '\n'
        << "contextLength=" << contextLength << '\n'
        << "generatedEvents=" << generatedNotes.size() << '\n'
        << "generatedPhraseValid=1\n"
        << "allPredictionsValid=1\n"
        << "allDecodedMidiEventsValid=1\n";

    return 0;
}
