#include "Music/CompositionConditionedSequenceNeuralTrainingService.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>

using namespace midigengx::music;

namespace
{

void expect(
    bool condition,
    const char* message)
{
    if (!condition)
    {
        std::cerr
            << "FAILED: "
            << message
            << '\n';
        std::exit(1);
    }
}

CompositionMidiTrainingSequence makeSequence(
    const std::string& id,
    double offset)
{
    CompositionMidiTrainingSequence sequence;
    sequence.sampleId = id;
    sequence.analysisValid = true;

    for (int index = 0;
         index < 80;
         ++index)
    {
        CompositionMidiTrainingEvent event;
        event.features.resize(
            CompositionMidiTrainingEvent::featureCount);

        for (std::size_t feature = 0;
             feature < event.features.size();
             ++feature)
        {
            event.features[feature] =
                std::clamp(
                    0.1 *
                        static_cast<double>(
                            (feature + index) % 5) /
                        5.0 +
                    offset,
                    -1.0,
                    1.0);
        }

        sequence.events.push_back(
            event);
    }

    return sequence;
}

CompositionSequenceMetadata makeMetadata(
    const std::string& id,
    const std::string& composer,
    const std::string& style,
    const std::string& era)
{
    CompositionSequenceMetadata metadata;

    metadata.sampleId = id;
    metadata.composerId = composer;
    metadata.workId = id + "_work";
    metadata.movementId = "movement_1";
    metadata.styleId = style;
    metadata.eraId = era;
    metadata.instrumentationId = "solo_piano";
    metadata.verified = true;
    metadata.valid = true;

    return metadata;
}

CompositionConditionedTrainingDataset buildDataset()
{
    const auto metadata =
        buildCompositionSequenceMetadataCatalog(
        {
            makeMetadata(
                "piece-a",
                "chopin",
                "romantic",
                "romantic"),
            makeMetadata(
                "piece-b",
                "bach",
                "baroque",
                "baroque"),
            makeMetadata(
                "piece-c",
                "chopin",
                "romantic",
                "romantic")
        });

    return buildCompositionConditionedTrainingDataset(
        {
            makeSequence("piece-a", 0.0),
            makeSequence("piece-b", 0.02),
            makeSequence("piece-c", 0.01)
        },
        metadata);
}

CompositionConditionedSequenceNeuralTrainingConfig config()
{
    CompositionConditionedSequenceNeuralTrainingConfig config;
    config.epochs = 10;
    config.learningRate = 0.001;
    config.gradientClip = 1.0;
    return config;
}

void testTrainingFromDatasetProducesModelArtifact()
{
    const auto dataset =
        buildDataset();

    expect(
        dataset.isValid(),
        "conditioned dataset fixture is valid");

    const auto result =
        trainCompositionConditionedSequenceNeuralModelFromDataset(
            dataset,
            config());

    expect(
        result.isValid(),
        "conditioned training service produces a valid trained artifact");

    expect(
        result.training.windowCount > 0,
        "conditioned training service creates windows");

    expect(
        result.training.finalLoss <
            result.training.initialLoss,
        "conditioned training service reduces loss");
}

void testModelArtifactRoundTripPreservesParameters()
{
    const auto dataset =
        buildDataset();

    const auto result =
        trainCompositionConditionedSequenceNeuralModelFromDataset(
            dataset,
            config());

    expect(
        result.isValid(),
        "trained conditioned artifact is valid");

    CompositionConditionedSequenceNeuralModel restored;

    expect(
        deserializeCompositionConditionedSequenceNeuralModel(
            result.artifact,
            restored),
        "conditioned model artifact deserializes");

    expect(
        restored.isValid(),
        "deserialized conditioned model is valid");

    const auto contract =
        buildCompositionSequenceLearningContract(
            64);

    const auto& sample =
        dataset.samples.front();

    const auto windows =
        buildCompositionMidiSequenceWindows(
            sample.sequence,
            contract);

    expect(
        !windows.empty(),
        "artifact round trip has sequence windows");

    const auto prediction =
        restored.predictNextEvent(
            windows.front(),
            sample);

    expect(
        prediction.isValid(
            contract.targetFeatureWidth),
        "deserialized conditioned model predicts valid event");
}

void testInvalidDatasetIsRejected()
{
    CompositionConditionedTrainingDataset invalid;

    const auto result =
        trainCompositionConditionedSequenceNeuralModelFromDataset(
            invalid,
            config());

    expect(
        !result.valid,
        "invalid conditioned dataset is rejected");
}

} // namespace

int main()
{
    testTrainingFromDatasetProducesModelArtifact();
    testModelArtifactRoundTripPreservesParameters();
    testInvalidDatasetIsRejected();

    std::cout
        << "MIDI-GenGX Phase 92 conditioned training service tests passed.\n";

    return 0;
}
