#include "Music/CompositionConditionedTrainingDatasetArtifact.h"

#include <cstdlib>
#include <iostream>

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

CompositionMidiTrainingSequence makeSequence(
    const std::string& id)
{
    CompositionMidiTrainingSequence sequence;

    sequence.sampleId = id;
    sequence.analysisValid = true;

    CompositionMidiTrainingEvent event;
    event.features.resize(
        CompositionMidiTrainingEvent::featureCount,
        0.0);

    sequence.events.push_back(
        event);

    return sequence;
}

CompositionSequenceMetadata makeMetadata(
    const std::string& id,
    const std::string& composer,
    const std::string& style)
{
    CompositionSequenceMetadata metadata;

    metadata.sampleId = id;
    metadata.composerId = composer;
    metadata.workId = id + "_work";
    metadata.movementId = "movement_1";
    metadata.styleId = style;
    metadata.eraId = "romantic";
    metadata.instrumentationId = "solo_piano";
    metadata.verified = true;
    metadata.valid = true;

    return metadata;
}

void testJoinBySampleId()
{
    const auto metadata =
        buildCompositionSequenceMetadataCatalog(
        {
            makeMetadata(
                "piece-b",
                "bach",
                "baroque"),
            makeMetadata(
                "piece-a",
                "chopin",
                "romantic")
        });

    const auto dataset =
        buildCompositionConditionedTrainingDataset(
        {
            makeSequence("piece-b"),
            makeSequence("piece-a")
        },
        metadata);

    expect(
        dataset.isValid(),
        "conditioned dataset joins valid sequence and metadata corpora");

    expect(
        dataset.sampleCount() == 2,
        "conditioned dataset preserves both samples");

    expect(
        dataset.samples[0].sequence.sampleId ==
            "piece-a" &&
        dataset.samples[0].metadata.composerId ==
            "chopin",
        "conditioned dataset is sorted deterministically");
}

void testVocabularyIsDeterministic()
{
    const auto metadata =
        buildCompositionSequenceMetadataCatalog(
        {
            makeMetadata(
                "a",
                "chopin",
                "romantic"),
            makeMetadata(
                "b",
                "bach",
                "baroque"),
            makeMetadata(
                "c",
                "chopin",
                "romantic")
        });

    const auto dataset =
        buildCompositionConditionedTrainingDataset(
        {
            makeSequence("a"),
            makeSequence("b"),
            makeSequence("c")
        },
        metadata);

    expect(
        dataset.isValid(),
        "vocabulary dataset is valid");

    expect(
        dataset.vocabulary.composers.size() == 2,
        "duplicate composers collapse into one vocabulary value");

    expect(
        dataset.vocabulary.composers[0] ==
            "bach" &&
        dataset.vocabulary.composers[1] ==
            "chopin",
        "composer vocabulary is sorted deterministically");
}

void testMissingMetadataRejectsTrainingDataset()
{
    const auto metadata =
        buildCompositionSequenceMetadataCatalog(
        {
            makeMetadata(
                "piece-a",
                "chopin",
                "romantic")
        });

    const auto dataset =
        buildCompositionConditionedTrainingDataset(
        {
            makeSequence("piece-a"),
            makeSequence("piece-b")
        },
        metadata);

    expect(
        !dataset.isValid(),
        "missing metadata prevents conditioned training dataset construction");
}

void testUnverifiedMetadataRejectsTrainingDataset()
{
    auto metadataEntry =
        makeMetadata(
            "piece-a",
            "chopin",
            "romantic");

    metadataEntry.verified = false;

    const auto metadata =
        buildCompositionSequenceMetadataCatalog(
        {
            metadataEntry
        });

    expect(
        metadata.isValid(),
        "unverified metadata catalog remains structurally valid");

    const auto dataset =
        buildCompositionConditionedTrainingDataset(
        {
            makeSequence("piece-a")
        },
        metadata);

    expect(
        !dataset.isValid(),
        "unverified metadata is excluded from training dataset");
}

void testConditionManifestIsDeterministic()
{
    const auto metadata =
        buildCompositionSequenceMetadataCatalog(
        {
            makeMetadata(
                "b",
                "bach",
                "baroque"),
            makeMetadata(
                "a",
                "chopin",
                "romantic")
        });

    const auto dataset =
        buildCompositionConditionedTrainingDataset(
        {
            makeSequence("b"),
            makeSequence("a")
        },
        metadata);

    const auto first =
        serializeCompositionConditionedTrainingDataset(
            dataset);

    const auto second =
        serializeCompositionConditionedTrainingDataset(
            dataset);

    expect(
        first.isValid() &&
        second.isValid(),
        "condition manifests are valid");

    expect(
        first.bytes ==
            second.bytes,
        "condition manifest serialization is deterministic");
}

} // namespace

int main()
{
    testJoinBySampleId();
    testVocabularyIsDeterministic();
    testMissingMetadataRejectsTrainingDataset();
    testUnverifiedMetadataRejectsTrainingDataset();
    testConditionManifestIsDeterministic();

    std::cout
        << "MIDI-GenGX Phase 90 conditioned dataset tests passed.\n";

    return 0;
}
