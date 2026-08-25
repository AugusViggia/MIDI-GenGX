#include "Music/CompositionSequenceNeuralTrainingService.h"

#include <cstdlib>
#include <iostream>
#include <vector>

using namespace midigengx::music;

namespace
{

void expect(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}

CompositionMidiTrainingSequence buildSequence(
    const std::string& id,
    double offset)
{
    CompositionMidiTrainingSequence sequence;
    sequence.sampleId = id;
    sequence.featureWidth =
        CompositionMidiTrainingEvent::featureCount;
    sequence.analysisValid = true;

    for (int index = 0; index < 80; ++index)
    {
        CompositionMidiTrainingEvent event;
        event.features.resize(
            CompositionMidiTrainingEvent::featureCount);

        for (std::size_t feature = 0;
             feature < event.features.size();
             ++feature)
        {
            event.features[feature] =
                0.1 *
                    static_cast<double>(
                        (feature + index) % 5) /
                    5.0 +
                offset;
        }

        sequence.events.push_back(event);
    }

    return sequence;
}

CompositionSequenceNeuralTrainingConfig config()
{
    CompositionSequenceNeuralTrainingConfig value;
    value.epochs = 12;
    value.learningRate = 0.001;
    value.gradientClip = 1.0;
    return value;
}

void testCorpusArtifactTrainingProducesModelArtifact()
{
    const std::vector<CompositionMidiTrainingSequence> sequences =
    {
        buildSequence("piece-a", 0.0),
        buildSequence("piece-b", 0.01),
        buildSequence("piece-c", 0.02)
    };

    const auto corpusArtifact =
        serializeCompositionMidiTrainingSequences(
            sequences);

    expect(
        corpusArtifact.isValid(),
        "sequence corpus artifact is valid");

    const auto result =
        trainSequenceNeuralModelFromArtifact(
            corpusArtifact,
            config());

    expect(
        result.isValid(),
        "sequence training service produces valid model artifact");

    expect(
        result.training.windowCount > 0,
        "sequence training service built training windows");

    expect(
        result.training.finalLoss <
            result.training.initialLoss,
        "sequence training service reduces loss");

    expect(
        result.artifact.isValid(),
        "trained sequence model artifact is valid");
}

void testTrainedArtifactRoundTripPreservesPredictions()
{
    const std::vector<CompositionMidiTrainingSequence> sequences =
    {
        buildSequence("piece-a", 0.0),
        buildSequence("piece-b", 0.02)
    };

    const auto corpusArtifact =
        serializeCompositionMidiTrainingSequences(
            sequences);

    const auto result =
        trainSequenceNeuralModelFromArtifact(
            corpusArtifact,
            config());

    expect(
        result.isValid(),
        "model training result is valid");

    CompositionSequenceNeuralModel restored;

    expect(
        deserializeCompositionSequenceNeuralModel(
            result.artifact,
            restored),
        "trained model artifact deserializes");

    const auto contract =
        buildCompositionSequenceLearningContract(
            64);

    const auto windows =
        buildCompositionMidiSequenceWindows(
            sequences.front(),
            contract);

    expect(
        !windows.empty(),
        "prediction round-trip has windows");

    const auto originalPrediction =
        [&]()
        {
            CompositionSequenceNeuralModel original;
            expect(
                deserializeCompositionSequenceNeuralModel(
                    result.artifact,
                    original),
                "original trained artifact reloads");

            return original.predictNextEvent(
                windows.front());
        }();

    const auto restoredPrediction =
        restored.predictNextEvent(
            windows.front());

    expect(
        originalPrediction.isValid(
            contract.targetFeatureWidth) &&
        restoredPrediction.isValid(
            contract.targetFeatureWidth),
        "round-trip predictions are valid");

    expect(
        originalPrediction.features ==
            restoredPrediction.features,
        "model artifact round trip preserves prediction exactly");
}

void testInvalidCorpusRejected()
{
    CompositionMidiTrainingCorpusArtifact invalid;

    const auto result =
        trainSequenceNeuralModelFromArtifact(
            invalid,
            config());

    expect(
        !result.valid,
        "invalid sequence corpus artifact is rejected");
}

} // namespace

int main()
{
    testCorpusArtifactTrainingProducesModelArtifact();
    testTrainedArtifactRoundTripPreservesPredictions();
    testInvalidCorpusRejected();

    std::cout
        << "MIDI-GenGX Phase 87 sequence training service tests passed.\n";

    return 0;
}
