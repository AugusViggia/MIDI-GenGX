#include "Music/CompositionTrainingCorpusArtifact.h"
#include "Music/MotifDevelopment.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

using namespace midigengx::music;
using namespace midigengx::domain;

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

CompositionDatasetPreparedView buildPrepared()
{
    MusicalContext context;
    context.parameters.lengthBars = 16;
    context.parameters.phraseLengthBars = 4;
    context.normalize();

    const auto structure =
        planPhraseStructure(
            context);

    const auto harmony =
        planHarmony(
            context,
            structure);

    const auto graph =
        buildCompositionKnowledgeGraph(
            structure,
            harmony);

    const auto transitions =
        analyzeCompositionTransitions(
            graph);

    Motif motif;
    motif.lengthBeats = 2.0;
    motif.notes =
    {
        MotifNote{0.0, 0.5, 0, 0},
        MotifNote{0.5, 0.5, 2, 1},
        MotifNote{1.0, 0.5, 5, -1},
        MotifNote{1.5, 0.5, 3, 0}
    };

    const auto motifGraph =
        buildMotifOccurrenceGraph(
            {
                motif,
                MotifDevelopment::transpose(
                    motif,
                    7)
            },
            {0, 8});

    const auto motifProfile =
        analyzeMotifRecurrence(
            motifGraph);

    const auto catalog =
        buildMotifKnowledgeCatalog(
            motifProfile);

    const auto composition =
        buildCompositionKnowledgeRecord(
            structure,
            harmony,
            catalog);

    const auto snapshot =
        buildCompositionKnowledgeSnapshot(
            composition,
            graph,
            transitions);

    std::vector<CompositionDatasetInput> inputs;

    for (int index = 0;
         index < 32;
         ++index)
    {
        inputs.push_back(
        {
            "corpus-artifact-" +
                std::to_string(index),
            snapshot
        });
    }

    const auto dataset =
        buildCompositionDataset(
            inputs);

    const auto quality =
        assessCompositionDatasetQuality(
            dataset);

    const auto partition =
        buildCompositionDatasetPartition(
            dataset,
            0.20,
            0.20);

    const auto manifest =
        buildCompositionDatasetManifest(
            dataset,
            quality,
            partition);

    return prepareCompositionDatasetForLearning(
        dataset,
        quality,
        manifest,
        partition);
}

void testValidCorpusArtifact()
{
    const auto prepared =
        buildPrepared();

    const auto artifact =
        serializeCompositionTrainingCorpus(
            prepared);

    expect(
        artifact.isValid(),
        "prepared corpus serializes to a valid artifact");

    expect(
        !artifact.bytes.empty(),
        "corpus artifact is non-empty");
}

void testCorpusRoundTripPreservesMatrices()
{
    const auto prepared =
        buildPrepared();

    const auto artifact =
        serializeCompositionTrainingCorpus(
            prepared);

    CompositionDatasetPreparedView restored;

    expect(
        deserializeCompositionTrainingCorpus(
            artifact,
            restored),
        "corpus artifact round-trip succeeds");

    expect(
        restored.normalizedBatch.globalMatrix ==
            prepared.normalizedBatch.globalMatrix,
        "round-trip preserves global matrix");

    expect(
        restored.normalizedBatch.sectionMatrix ==
            prepared.normalizedBatch.sectionMatrix,
        "round-trip preserves section matrix");

    expect(
        restored.normalizedBatch.sectionMask ==
            prepared.normalizedBatch.sectionMask,
        "round-trip preserves section mask");
}

void testCorpusShapeIsPreserved()
{
    const auto prepared =
        buildPrepared();

    const auto artifact =
        serializeCompositionTrainingCorpus(
            prepared);

    CompositionDatasetPreparedView restored;

    expect(
        deserializeCompositionTrainingCorpus(
            artifact,
            restored),
        "shape round-trip succeeds");

    expect(
        restored.normalizedBatch.sampleCount ==
            prepared.normalizedBatch.sampleCount &&
        restored.normalizedBatch.maxSectionCount ==
            prepared.normalizedBatch.maxSectionCount,
        "corpus shape metadata is preserved");

    expect(
        restored.normalizedBatch.globalFeatureWidth ==
            CompositionDatasetSchema::globalFeatureCount &&
        restored.normalizedBatch.sectionFeatureWidth ==
            CompositionDatasetSchema::sectionFeatureCount,
        "corpus schema dimensions are preserved");
}

void testCorruptedCorpusArtifactIsRejected()
{
    const auto prepared =
        buildPrepared();

    auto artifact =
        serializeCompositionTrainingCorpus(
            prepared);

    artifact.bytes[0] ^=
        static_cast<std::uint8_t>(
            0xFF);

    CompositionDatasetPreparedView restored;

    expect(
        !deserializeCompositionTrainingCorpus(
            artifact,
            restored),
        "corrupted corpus artifact is rejected");
}

void testTruncatedCorpusArtifactIsRejected()
{
    const auto prepared =
        buildPrepared();

    auto artifact =
        serializeCompositionTrainingCorpus(
            prepared);

    artifact.bytes.pop_back();

    expect(
        !artifact.isValid(),
        "truncated corpus artifact is rejected");
}

void testDeterministicCorpusArtifact()
{
    const auto prepared =
        buildPrepared();

    const auto first =
        serializeCompositionTrainingCorpus(
            prepared);

    const auto second =
        serializeCompositionTrainingCorpus(
            prepared);

    expect(
        first.bytes ==
            second.bytes,
        "identical prepared corpus produces identical artifact");
}

void testInvalidPreparedViewIsRejected()
{
    CompositionDatasetPreparedView invalid;

    const auto artifact =
        serializeCompositionTrainingCorpus(
            invalid);

    expect(
        artifact.bytes.empty(),
        "invalid prepared corpus is not serialized");
}

} // namespace

int main()
{
    testValidCorpusArtifact();
    testCorpusRoundTripPreservesMatrices();
    testCorpusShapeIsPreserved();
    testCorruptedCorpusArtifactIsRejected();
    testTruncatedCorpusArtifactIsRejected();
    testDeterministicCorpusArtifact();
    testInvalidPreparedViewIsRejected();

    std::cout
        << "MIDI-GenGX Composition Training Corpus Artifact tests passed.\n";

    return 0;
}
