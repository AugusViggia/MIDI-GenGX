#include "Music/CompositionDatasetManifest.h"
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

CompositionDataset buildDataset(
    int count)
{
    const auto snapshot =
        buildSnapshot();

    std::vector<CompositionDatasetInput>
        inputs;

    for (int index = 0;
         index < count;
         ++index)
    {
        inputs.push_back(
        {
            "manifest-" +
                std::to_string(index),
            snapshot
        });
    }

    return buildCompositionDataset(
        inputs);
}

CompositionDatasetPartition buildPartition(
    const CompositionDataset& dataset)
{
    return buildCompositionDatasetPartition(
        dataset,
        0.20,
        0.20);
}

void testManifestValidity()
{
    const auto dataset =
        buildDataset(20);

    const auto quality =
        assessCompositionDatasetQuality(
            dataset);

    const auto partition =
        buildPartition(
            dataset);

    const auto manifest =
        buildCompositionDatasetManifest(
            dataset,
            quality,
            partition);

    expect(
        manifest.isValid(),
        "dataset manifest is valid");

    expect(
        manifest.schemaVersion ==
            CompositionDatasetSchema::version,
        "manifest carries schema version");

    expect(
        manifest.sampleCount ==
            dataset.size(),
        "manifest carries sample count");

    expect(
        manifest.trainingCount +
            manifest.validationCount +
            manifest.testCount ==
            manifest.sampleCount,
        "manifest split counts cover dataset");
}

void testStableSignature()
{
    const auto datasetA =
        buildDataset(12);

    const auto datasetB =
        buildDataset(12);

    const auto qualityA =
        assessCompositionDatasetQuality(
            datasetA);

    const auto qualityB =
        assessCompositionDatasetQuality(
            datasetB);

    const auto partitionA =
        buildPartition(
            datasetA);

    const auto partitionB =
        buildPartition(
            datasetB);

    const auto manifestA =
        buildCompositionDatasetManifest(
            datasetA,
            qualityA,
            partitionA);

    const auto manifestB =
        buildCompositionDatasetManifest(
            datasetB,
            qualityB,
            partitionB);

    expect(
        manifestA.datasetSignature ==
            manifestB.datasetSignature,
        "identical datasets produce identical signatures");

    expect(
        manifestA.signatureHex() ==
            manifestB.signatureHex(),
        "signature hexadecimal representation is stable");
}

void testSampleChangeChangesSignature()
{
    const auto original =
        buildDataset(8);

    auto changed =
        original;

    changed.samples[0]
        .globalFeatures[0] +=
            0.001;

    const auto originalQuality =
        assessCompositionDatasetQuality(
            original);

    const auto changedQuality =
        assessCompositionDatasetQuality(
            changed);

    const auto originalPartition =
        buildPartition(
            original);

    const auto changedPartition =
        buildPartition(
            changed);

    const auto originalManifest =
        buildCompositionDatasetManifest(
            original,
            originalQuality,
            originalPartition);

    const auto changedManifest =
        buildCompositionDatasetManifest(
            changed,
            changedQuality,
            changedPartition);

    expect(
        originalManifest.datasetSignature !=
            changedManifest.datasetSignature,
        "dataset content changes signature");
}

void testInvalidQualityProducesInvalidManifest()
{
    const auto dataset =
        buildDataset(6);

    auto quality =
        assessCompositionDatasetQuality(
            dataset);

    quality.globalFeatureWidth = 1;

    const auto partition =
        buildPartition(
            dataset);

    const auto manifest =
        buildCompositionDatasetManifest(
            dataset,
            quality,
            partition);

    expect(
        !manifest.analysisValid,
        "invalid quality prevents manifest creation");

    expect(
        !manifest.isValid(),
        "invalid quality produces invalid manifest");
}

void testEmptyDatasetManifest()
{
    const auto dataset =
        buildDataset(0);

    const auto quality =
        assessCompositionDatasetQuality(
            dataset);

    const auto partition =
        buildPartition(
            dataset);

    const auto manifest =
        buildCompositionDatasetManifest(
            dataset,
            quality,
            partition);

    expect(
        manifest.analysisValid,
        "empty dataset can produce analyzed manifest");

    expect(
        manifest.isValid(),
        "empty dataset manifest is valid");

    expect(
        manifest.sampleCount == 0 &&
        manifest.datasetSignature == 0,
        "empty dataset manifest has zero signature");

    expect(
        manifest.globalFeatureWidth ==
            CompositionDatasetSchema::globalFeatureCount &&
        manifest.sectionFeatureWidth ==
            CompositionDatasetSchema::sectionFeatureCount,
        "empty manifest still declares the dataset schema");
}

void testSchemaWidthsAreCarried()
{
    const auto dataset =
        buildDataset(5);

    const auto quality =
        assessCompositionDatasetQuality(
            dataset);

    const auto partition =
        buildPartition(
            dataset);

    const auto manifest =
        buildCompositionDatasetManifest(
            dataset,
            quality,
            partition);

    expect(
        manifest.globalFeatureWidth ==
            CompositionDatasetSchema::globalFeatureCount,
        "manifest carries global feature width");

    expect(
        manifest.sectionFeatureWidth ==
            CompositionDatasetSchema::sectionFeatureCount,
        "manifest carries section feature width");
}

} // namespace

int main()
{
    testManifestValidity();
    testStableSignature();
    testSampleChangeChangesSignature();
    testInvalidQualityProducesInvalidManifest();
    testEmptyDatasetManifest();
    testSchemaWidthsAreCarried();

    std::cout
        << "MIDI-GenGX Composition Dataset Manifest tests passed.\n";

    return 0;
}
