#include "Music/CompositionDatasetNormalization.h"
#include "Music/MotifDevelopment.h"

#include <cmath>
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
            {"norm-a", snapshot},
            {"norm-b", snapshot},
            {"norm-c", snapshot},
            {"norm-d", snapshot},
            {"norm-e", snapshot}
        });
}

CompositionDatasetBatch buildBatch(
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

    const auto manifest =
        buildCompositionDatasetManifest(
            dataset,
            quality,
            partition);

    return buildCompositionDatasetBatch(
        dataset,
        manifest);
}

CompositionDatasetPartition buildPartition(
    const CompositionDataset& dataset)
{
    return buildCompositionDatasetPartition(
        dataset,
        0.20,
        0.20);
}

void testTrainingOnlyFit()
{
    const auto dataset =
        buildDataset();

    const auto batch =
        buildBatch(dataset);

    const auto partition =
        buildPartition(dataset);

    const auto normalization =
        fitCompositionDatasetNormalization(
            batch,
            partition);

    expect(
        normalization.isValid(),
        "normalization profile is valid");

    expect(
        normalization.globalMean.size() == 13 &&
        normalization.globalStdDev.size() == 13,
        "global normalization width is stable");

    expect(
        normalization.sectionMean.size() == 6 &&
        normalization.sectionStdDev.size() == 6,
        "section normalization width is stable");
}

void testNoDataLeakageIntoStatistics()
{
    auto dataset =
        buildDataset();

    const auto batch =
        buildBatch(dataset);

    const auto partition =
        buildPartition(dataset);

    const auto baseline =
        fitCompositionDatasetNormalization(
            batch,
            partition);

    auto modified =
        batch;

    for (const auto index :
         partition.validationIndices)
    {
        modified.globalMatrix[
            index *
                modified.globalFeatureWidth] =
            0.999999;
    }

    for (const auto index :
         partition.testIndices)
    {
        modified.globalMatrix[
            index *
                modified.globalFeatureWidth +
            1] =
            0.000001;
    }

    const auto after =
        fitCompositionDatasetNormalization(
            modified,
            partition);

    expect(
        baseline.globalMean ==
            after.globalMean,
        "validation/test data cannot alter global means");

    expect(
        baseline.globalStdDev ==
            after.globalStdDev,
        "validation/test data cannot alter global standard deviations");
}

void testNormalizationProducesFiniteValues()
{
    const auto dataset =
        buildDataset();

    const auto batch =
        buildBatch(dataset);

    const auto partition =
        buildPartition(dataset);

    const auto normalization =
        fitCompositionDatasetNormalization(
            batch,
            partition);

    const auto normalized =
        applyCompositionDatasetNormalization(
            batch,
            normalization);

    expect(
        normalized.isValid(),
        "normalized batch remains structurally valid");

    for (const auto value :
         normalized.globalMatrix)
    {
        expect(
            std::isfinite(value),
            "normalized global values are finite");
    }

    for (const auto value :
         normalized.sectionMatrix)
    {
        expect(
            std::isfinite(value),
            "normalized section values are finite");
    }
}

void testPaddingRemainsZeroAndMasked()
{
    const auto dataset =
        buildDataset();

    const auto batch =
        buildBatch(dataset);

    const auto partition =
        buildPartition(dataset);

    const auto normalization =
        fitCompositionDatasetNormalization(
            batch,
            partition);

    const auto normalized =
        applyCompositionDatasetNormalization(
            batch,
            normalization);

    for (std::size_t sampleIndex = 0;
         sampleIndex < normalized.sampleCount;
         ++sampleIndex)
    {
        const auto sectionCount =
            dataset.samples[sampleIndex]
                .sectionCount();

        for (std::size_t sectionIndex =
                 sectionCount;
             sectionIndex <
                 normalized.maxSectionCount;
             ++sectionIndex)
        {
            const auto base =
                (sampleIndex *
                     normalized.maxSectionCount +
                 sectionIndex) *
                normalized.sectionFeatureWidth;

            expect(
                normalized.sectionMask[
                    sampleIndex *
                        normalized.maxSectionCount +
                    sectionIndex] == 0.0,
                "padding mask remains zero");

            for (std::size_t featureIndex = 0;
                 featureIndex <
                     normalized.sectionFeatureWidth;
                 ++featureIndex)
            {
                expect(
                    normalized.sectionMatrix[
                        base + featureIndex] == 0.0,
                    "padding values remain zero");
            }
        }
    }
}

void testConstantFeaturesUseStableUnitScale()
{
    const auto dataset =
        buildDataset();

    const auto batch =
        buildBatch(dataset);

    const auto partition =
        buildPartition(dataset);

    const auto normalization =
        fitCompositionDatasetNormalization(
            batch,
            partition);

    for (const auto stddev :
         normalization.globalStdDev)
    {
        expect(
            stddev > 0.0,
            "global constant features get a stable unit scale");
    }

    for (const auto stddev :
         normalization.sectionStdDev)
    {
        expect(
            stddev > 0.0,
            "section constant features get a stable unit scale");
    }
}

void testInvalidInputsAreRejected()
{
    CompositionDatasetBatch invalidBatch;
    CompositionDatasetPartition invalidPartition;

    const auto normalization =
        fitCompositionDatasetNormalization(
            invalidBatch,
            invalidPartition);

    expect(
        !normalization.analysisValid,
        "invalid normalization inputs are rejected");

    expect(
        !normalization.isValid(),
        "invalid normalization profile is invalid");
}

} // namespace

int main()
{
    testTrainingOnlyFit();
    testNoDataLeakageIntoStatistics();
    testNormalizationProducesFiniteValues();
    testPaddingRemainsZeroAndMasked();
    testConstantFeaturesUseStableUnitScale();
    testInvalidInputsAreRejected();

    std::cout
        << "MIDI-GenGX Composition Dataset Normalization tests passed.\n";

    return 0;
}
