#include "Music/CompositionConditionedTrainingRunService.h"

#include <algorithm>
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
    metadata.workId =
        id + "_work";
    metadata.movementId =
        "movement_1";
    metadata.styleId =
        style;
    metadata.eraId =
        era;
    metadata.instrumentationId =
        "solo_piano";
    metadata.verified = true;
    metadata.valid = true;
    return metadata;
}

CompositionMidiTrainingCorpusArtifact
makeSequenceArtifact()
{
    return serializeCompositionMidiTrainingSequences(
        {
            makeSequence(
                "piece-a",
                0.00),
            makeSequence(
                "piece-b",
                0.02),
            makeSequence(
                "piece-c",
                0.01)
        });
}

CompositionSequenceMetadataArtifact
makeMetadataArtifact()
{
    const auto catalog =
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

    return serializeCompositionSequenceMetadataCatalog(
        catalog);
}

CompositionConditionedSequenceNeuralTrainingConfig
makeConfig()
{
    CompositionConditionedSequenceNeuralTrainingConfig config;
    config.epochs = 10;
    config.learningRate = 0.001;
    config.gradientClip = 1.0;
    return config;
}

void testEndToEndConditionedTraining()
{
    const auto sequenceArtifact =
        makeSequenceArtifact();

    const auto metadataArtifact =
        makeMetadataArtifact();

    expect(
        sequenceArtifact.isValid(),
        "sequence corpus artifact fixture is valid");

    expect(
        metadataArtifact.isValid(),
        "metadata artifact fixture is valid");

    const auto result =
        runCompositionConditionedTraining(
            sequenceArtifact,
            metadataArtifact,
            makeConfig());

    expect(
        result.isValid(),
        "conditioned training run completes");

    expect(
        result.sequenceCount == 3 &&
        result.conditionedSampleCount == 3,
        "training run preserves all joined samples");

    expect(
        result.training.finalLoss <
            result.training.initialLoss,
        "real conditioned training reduces loss");

    expect(
        result.datasetArtifact.isValid(),
        "conditioned dataset manifest is persisted");

    expect(
        result.modelArtifact.isValid(),
        "trained conditioned model artifact is persisted");
}

void testMissingMetadataFailsClosed()
{
    const auto sequenceArtifact =
        makeSequenceArtifact();

    const auto incompleteCatalog =
        buildCompositionSequenceMetadataCatalog(
        {
            makeMetadata(
                "piece-a",
                "chopin",
                "romantic",
                "romantic")
        });

    const auto metadataArtifact =
        serializeCompositionSequenceMetadataCatalog(
            incompleteCatalog);

    const auto result =
        runCompositionConditionedTraining(
            sequenceArtifact,
            metadataArtifact,
            makeConfig());

    expect(
        !result.valid,
        "missing metadata fails the conditioned training run closed");
}

void testUnverifiedMetadataFailsClosed()
{
    auto metadata =
        makeMetadata(
            "piece-a",
            "chopin",
            "romantic",
            "romantic");

    metadata.verified = false;

    const auto catalog =
        buildCompositionSequenceMetadataCatalog(
        {
            metadata,
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

    const auto metadataArtifact =
        serializeCompositionSequenceMetadataCatalog(
            catalog);

    const auto result =
        runCompositionConditionedTraining(
            makeSequenceArtifact(),
            metadataArtifact,
            makeConfig());

    expect(
        !result.valid,
        "unverified metadata fails the conditioned training run closed");
}

} // namespace

int main()
{
    testEndToEndConditionedTraining();
    testMissingMetadataFailsClosed();
    testUnverifiedMetadataFailsClosed();

    std::cout
        << "MIDI-GenGX Phase 93 conditioned training run service tests passed.\n";

    return 0;
}
