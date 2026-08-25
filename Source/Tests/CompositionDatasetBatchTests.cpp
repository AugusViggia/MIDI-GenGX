#include "Music/CompositionDatasetBatch.h"
#include "Music/MotifDevelopment.h"

#include <cstdlib>
#include <iostream>
#include <string>
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

Motif makeMotif()
{
    Motif motif;
    motif.lengthBeats = 2.0;
    motif.notes =
    {
        MotifNote{0.0, 0.5, 0, 0},
        MotifNote{0.5, 0.5, 2, 1},
        MotifNote{1.0, 0.5, 5, -1},
        MotifNote{1.5, 0.5, 3, 0}
    };
    return motif;
}

CompositionKnowledgeSnapshot buildSnapshot()
{
    midigengx::domain::MusicalContext context;

    context.parameters.lengthBars = 16;
    context.parameters.phraseLengthBars = 4;
    context.normalize();

    const auto structure =
        planPhraseStructure(context);

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

    const auto motifGraph =
        buildMotifOccurrenceGraph(
            {
                makeMotif(),
                MotifDevelopment::transpose(
                    makeMotif(),
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

    return buildCompositionKnowledgeSnapshot(
        composition,
        graph,
        transitions);
}

CompositionDataset buildDataset()
{
    const auto snapshot =
        buildSnapshot();

    return buildCompositionDataset(
        {
            {"batch-a", snapshot},
            {"batch-b", snapshot},
            {"batch-c", snapshot}
        });
}

CompositionDatasetManifest buildManifest(
    const CompositionDataset& dataset)
{
    const auto quality =
        assessCompositionDatasetQuality(
            dataset);

    const auto partition =
        buildCompositionDatasetPartition(
            dataset,
            0.20,
            0.20);

    return buildCompositionDatasetManifest(
        dataset,
        quality,
        partition);
}

void testBatchShape()
{
    const auto dataset =
        buildDataset();

    const auto manifest =
        buildManifest(
            dataset);

    const auto batch =
        buildCompositionDatasetBatch(
            dataset,
            manifest);

    expect(
        batch.isValid(),
        "dataset batch is valid");

    expect(
        batch.sampleCount ==
            dataset.size(),
        "batch preserves sample count");

    expect(
        batch.globalFeatureWidth == 13 &&
        batch.sectionFeatureWidth == 6,
        "batch preserves schema widths");

    expect(
        batch.globalMatrixSize() ==
            dataset.size() * 13,
        "global matrix has expected size");

    expect(
        batch.sectionMatrixSize() ==
            dataset.size() *
            batch.maxSectionCount *
            6,
        "section matrix has expected padded size");
}

void testSectionMask()
{
    const auto dataset =
        buildDataset();

    const auto manifest =
        buildManifest(
            dataset);

    const auto batch =
        buildCompositionDatasetBatch(
            dataset,
            manifest);

    expect(
        batch.sectionMask.size() ==
            dataset.size() *
            batch.maxSectionCount,
        "section mask has expected size");

    for (std::size_t sampleIndex = 0;
         sampleIndex < dataset.size();
         ++sampleIndex)
    {
        const auto sectionCount =
            dataset.samples[sampleIndex]
                .sectionCount();

        for (std::size_t sectionIndex = 0;
             sectionIndex <
                 batch.maxSectionCount;
             ++sectionIndex)
        {
            const auto mask =
                batch.sectionMask[
                    sampleIndex *
                        batch.maxSectionCount +
                    sectionIndex];

            expect(
                (sectionIndex < sectionCount &&
                 mask == 1.0) ||
                (sectionIndex >= sectionCount &&
                 mask == 0.0),
                "section mask matches real/padded sections");
        }
    }
}

void testPaddingIsZero()
{
    const auto dataset =
        buildDataset();

    const auto manifest =
        buildManifest(
            dataset);

    const auto batch =
        buildCompositionDatasetBatch(
            dataset,
            manifest);

    for (std::size_t sampleIndex = 0;
         sampleIndex < dataset.size();
         ++sampleIndex)
    {
        const auto sectionCount =
            dataset.samples[sampleIndex]
                .sectionCount();

        for (std::size_t sectionIndex =
                 sectionCount;
             sectionIndex <
                 batch.maxSectionCount;
             ++sectionIndex)
        {
            const auto base =
                (sampleIndex *
                     batch.maxSectionCount +
                 sectionIndex) *
                batch.sectionFeatureWidth;

            for (std::size_t featureIndex = 0;
                 featureIndex <
                     batch.sectionFeatureWidth;
                 ++featureIndex)
            {
                expect(
                    batch.sectionMatrix[
                        base + featureIndex] == 0.0,
                    "padded section features are zero");
            }
        }
    }
}

void testDeterministicBatchEncoding()
{
    const auto dataset =
        buildDataset();

    const auto manifest =
        buildManifest(
            dataset);

    const auto first =
        buildCompositionDatasetBatch(
            dataset,
            manifest);

    const auto second =
        buildCompositionDatasetBatch(
            dataset,
            manifest);

    expect(
        first.globalMatrix ==
            second.globalMatrix,
        "global batch encoding is deterministic");

    expect(
        first.sectionMatrix ==
            second.sectionMatrix,
        "section batch encoding is deterministic");

    expect(
        first.sectionMask ==
            second.sectionMask,
        "section mask encoding is deterministic");
}

void testInvalidManifestIsRejected()
{
    const auto dataset =
        buildDataset();

    CompositionDatasetManifest invalid;

    const auto batch =
        buildCompositionDatasetBatch(
            dataset,
            invalid);

    expect(
        !batch.analysisValid,
        "invalid manifest prevents batch construction");

    expect(
        !batch.isValid(),
        "invalid manifest produces invalid batch");
}

void testEmptyDatasetProducesEmptyBatch()
{
    const auto dataset =
        buildCompositionDataset({});

    const auto manifest =
        buildManifest(
            dataset);

    const auto batch =
        buildCompositionDatasetBatch(
            dataset,
            manifest);

    expect(
        batch.isValid(),
        "empty dataset produces valid empty batch");

    expect(
        batch.sampleCount == 0 &&
        batch.globalMatrix.empty() &&
        batch.sectionMatrix.empty() &&
        batch.sectionMask.empty(),
        "empty batch contains no feature data");
}

} // namespace

int main()
{
    testBatchShape();
    testSectionMask();
    testPaddingIsZero();
    testDeterministicBatchEncoding();
    testInvalidManifestIsRejected();
    testEmptyDatasetProducesEmptyBatch();

    std::cout
        << "MIDI-GenGX Composition Dataset Batch tests passed.\n";

    return 0;
}
