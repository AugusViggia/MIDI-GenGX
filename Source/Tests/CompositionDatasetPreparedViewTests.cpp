#include "Music/CompositionDatasetPreparedView.h"
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

CompositionDataset buildDataset(
    std::size_t count)
{
    const auto snapshot =
        buildSnapshot();

    std::vector<CompositionDatasetInput>
        inputs;

    for (std::size_t index = 0;
         index < count;
         ++index)
    {
        inputs.push_back(
        {
            "prepared-" +
                std::to_string(index),
            snapshot
        });
    }

    return buildCompositionDataset(
        inputs);
}

struct PreparedInputs
{
    CompositionDatasetQuality quality;
    CompositionDatasetPartition partition;
    CompositionDatasetManifest manifest;
};

PreparedInputs buildInputs(
    const CompositionDataset& dataset)
{
    PreparedInputs inputs;

    inputs.quality =
        assessCompositionDatasetQuality(
            dataset);

    inputs.partition =
        buildCompositionDatasetPartition(
            dataset,
            0.20,
            0.20);

    inputs.manifest =
        buildCompositionDatasetManifest(
            dataset,
            inputs.quality,
            inputs.partition);

    return inputs;
}

void testPreparationIsValid()
{
    const auto dataset =
        buildDataset(20);

    const auto inputs =
        buildInputs(dataset);

    const auto prepared =
        prepareCompositionDatasetForLearning(
            dataset,
            inputs.quality,
            inputs.manifest,
            inputs.partition);

    expect(
        prepared.analysisValid,
        "prepared learning view is analyzed");

    expect(
        prepared.isValid(),
        "prepared learning view is valid");

    expect(
        prepared.trainingCount() ==
            inputs.partition.trainingCount(),
        "prepared training count matches partition");

    expect(
        prepared.validationCount() ==
            inputs.partition.validationCount(),
        "prepared validation count matches partition");

    expect(
        prepared.testCount() ==
            inputs.partition.testCount(),
        "prepared test count matches partition");
}

void testNormalizationIsFittedBeforeLearningViewIsExposed()
{
    const auto dataset =
        buildDataset(20);

    const auto inputs =
        buildInputs(dataset);

    const auto prepared =
        prepareCompositionDatasetForLearning(
            dataset,
            inputs.quality,
            inputs.manifest,
            inputs.partition);

    expect(
        prepared.normalization.isValid(),
        "learning view contains valid preprocessing parameters");

    expect(
        prepared.normalizedBatch.isValid(),
        "learning view contains valid normalized batch");

    expect(
        prepared.partition.isValid(
            dataset.size()),
        "learning view preserves valid partition");
}

void testPartitionIndicesRemainAligned()
{
    const auto dataset =
        buildDataset(20);

    const auto inputs =
        buildInputs(dataset);

    const auto prepared =
        prepareCompositionDatasetForLearning(
            dataset,
            inputs.quality,
            inputs.manifest,
            inputs.partition);

    expect(
        prepared.partition.trainingIndices ==
            inputs.partition.trainingIndices,
        "training indices remain unchanged");

    expect(
        prepared.partition.validationIndices ==
            inputs.partition.validationIndices,
        "validation indices remain unchanged");

    expect(
        prepared.partition.testIndices ==
            inputs.partition.testIndices,
        "test indices remain unchanged");
}

void testPreparedDataIsDeterministic()
{
    const auto dataset =
        buildDataset(20);

    const auto inputs =
        buildInputs(dataset);

    const auto first =
        prepareCompositionDatasetForLearning(
            dataset,
            inputs.quality,
            inputs.manifest,
            inputs.partition);

    const auto second =
        prepareCompositionDatasetForLearning(
            dataset,
            inputs.quality,
            inputs.manifest,
            inputs.partition);

    expect(
        first.normalizedBatch.globalMatrix ==
            second.normalizedBatch.globalMatrix,
        "prepared global matrix is deterministic");

    expect(
        first.normalizedBatch.sectionMatrix ==
            second.normalizedBatch.sectionMatrix,
        "prepared section matrix is deterministic");

    expect(
        first.normalization.globalMean ==
            second.normalization.globalMean &&
        first.normalization.globalStdDev ==
            second.normalization.globalStdDev,
        "prepared normalization parameters are deterministic");
}

void testInvalidManifestIsRejected()
{
    const auto dataset =
        buildDataset(10);

    const auto quality =
        assessCompositionDatasetQuality(
            dataset);

    const auto partition =
        buildCompositionDatasetPartition(
            dataset,
            0.20,
            0.20);

    CompositionDatasetManifest invalid;

    const auto prepared =
        prepareCompositionDatasetForLearning(
            dataset,
            quality,
            invalid,
            partition);

    expect(
        !prepared.analysisValid,
        "invalid manifest prevents learning preparation");

    expect(
        !prepared.isValid(),
        "invalid manifest produces invalid learning view");
}

void testInvalidPartitionIsRejected()
{
    const auto dataset =
        buildDataset(10);

    const auto quality =
        assessCompositionDatasetQuality(
            dataset);

    const auto manifest =
        buildCompositionDatasetManifest(
            dataset,
            quality,
            buildCompositionDatasetPartition(
                dataset,
                0.20,
                0.20));

    CompositionDatasetPartition invalid;

    const auto prepared =
        prepareCompositionDatasetForLearning(
            dataset,
            quality,
            manifest,
            invalid);

    expect(
        !prepared.analysisValid,
        "invalid partition prevents learning preparation");
}

void testEmptyDatasetIsAValidPreparedState()
{
    const auto dataset =
        buildDataset(0);

    const auto inputs =
        buildInputs(dataset);

    const auto prepared =
        prepareCompositionDatasetForLearning(
            dataset,
            inputs.quality,
            inputs.manifest,
            inputs.partition);

    expect(
        prepared.analysisValid,
        "empty dataset produces analyzed learning view");

    expect(
        prepared.isValid(),
        "empty learning view is valid");

    expect(
        prepared.trainingCount() == 0 &&
        prepared.validationCount() == 0 &&
        prepared.testCount() == 0,
        "empty learning view has zero split counts");
}

} // namespace

int main()
{
    testPreparationIsValid();
    testNormalizationIsFittedBeforeLearningViewIsExposed();
    testPartitionIndicesRemainAligned();
    testPreparedDataIsDeterministic();
    testInvalidManifestIsRejected();
    testInvalidPartitionIsRejected();
    testEmptyDatasetIsAValidPreparedState();

    std::cout
        << "MIDI-GenGX Composition Dataset Prepared View tests passed.\n";

    return 0;
}
