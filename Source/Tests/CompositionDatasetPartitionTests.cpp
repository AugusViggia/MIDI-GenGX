#include "Music/CompositionDatasetPartition.h"
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

    std::vector<CompositionDatasetInput> inputs;

    for (int index = 0;
         index < 40;
         ++index)
    {
        inputs.push_back(
        {
            "composition-" +
                std::to_string(index),
            snapshot
        });
    }

    return buildCompositionDataset(
        inputs);
}

void testPartitionCoversDatasetExactlyOnce()
{
    const auto dataset =
        buildDataset();

    const auto partition =
        buildCompositionDatasetPartition(
            dataset,
            0.20,
            0.20);

    expect(
        partition.isValid(
            dataset.size()),
        "dataset partition is valid");

    expect(
        partition.trainingCount() +
            partition.validationCount() +
            partition.testCount() ==
            dataset.size(),
        "partition covers every sample exactly once");
}

void testPartitionIsDeterministic()
{
    const auto dataset =
        buildDataset();

    const auto first =
        buildCompositionDatasetPartition(
            dataset,
            0.20,
            0.20);

    const auto second =
        buildCompositionDatasetPartition(
            dataset,
            0.20,
            0.20);

    expect(
        first.trainingIndices ==
            second.trainingIndices &&
        first.validationIndices ==
            second.validationIndices &&
        first.testIndices ==
            second.testIndices,
        "partitioning is deterministic");
}

void testPartitionIsIndependentOfInputConstructionOrder()
{
    const auto snapshot =
        buildSnapshot();

    std::vector<CompositionDatasetInput>
        forward;

    std::vector<CompositionDatasetInput>
        reverse;

    for (int index = 0;
         index < 30;
         ++index)
    {
        const auto id =
            "stable-" +
            std::to_string(index);

        forward.push_back(
        {
            id,
            snapshot
        });
    }

    for (int index = 29;
         index >= 0;
         --index)
    {
        reverse.push_back(
        {
            "stable-" +
                std::to_string(index),
            snapshot
        });
    }

    const auto datasetA =
        buildCompositionDataset(
            forward);

    const auto datasetB =
        buildCompositionDataset(
            reverse);

    const auto partitionA =
        buildCompositionDatasetPartition(
            datasetA,
            0.20,
            0.20);

    const auto partitionB =
        buildCompositionDatasetPartition(
            datasetB,
            0.20,
            0.20);

    expect(
        partitionA.trainingCount() ==
            partitionB.trainingCount() &&
        partitionA.validationCount() ==
            partitionB.validationCount() &&
        partitionA.testCount() ==
            partitionB.testCount(),
        "same sample ids produce stable split counts");

    for (const auto* sample :
         {
             datasetA.findById("stable-3"),
             datasetA.findById("stable-17"),
             datasetA.findById("stable-28")
         })
    {
        expect(
            sample != nullptr,
            "stable sample exists in dataset A");
    }
}

void testInvalidRatiosAreRejected()
{
    const auto dataset =
        buildDataset();

    for (const auto ratios :
         {
             std::pair<double, double>{-0.1, 0.2},
             std::pair<double, double>{0.2, -0.1},
             std::pair<double, double>{1.0, 0.0},
             std::pair<double, double>{0.0, 1.0},
             std::pair<double, double>{0.6, 0.4}
         })
    {
        const auto partition =
            buildCompositionDatasetPartition(
                dataset,
                ratios.first,
                ratios.second);

        expect(
            !partition.analysisValid,
            "invalid split ratios are rejected");
    }
}

void testInvalidDatasetIsRejected()
{
    CompositionDataset dataset;

    CompositionDatasetPartition partition =
        buildCompositionDatasetPartition(
            dataset,
            0.20,
            0.20);

    // An empty dataset is valid and partitions to empty sets.
    expect(
        partition.analysisValid &&
        partition.isValid(0),
        "empty dataset has valid empty partition");

    CompositionDatasetSample invalid;
    invalid.analysisValid = true;
    invalid.sampleId = "broken";

    dataset.samples.push_back(
        invalid);

    partition =
        buildCompositionDatasetPartition(
            dataset,
            0.20,
            0.20);

    expect(
        !partition.analysisValid,
        "invalid dataset cannot be partitioned");
}

void testPartitionRatiosProduceExpectedCounts()
{
    const auto dataset =
        buildDataset();

    const auto partition =
        buildCompositionDatasetPartition(
            dataset,
            0.25,
            0.25);

    expect(
        partition.trainingCount() == 20,
        "training split receives expected count");

    expect(
        partition.validationCount() == 10,
        "validation split receives expected count");

    expect(
        partition.testCount() == 10,
        "test split receives expected count");

    expect(
        partition.trainingCount() +
            partition.validationCount() +
            partition.testCount() ==
            dataset.size(),
        "all split counts sum to dataset size");
}

void testSmallDatasetKeepsRequestedSplitsPopulated()
{
    const auto snapshot =
        buildSnapshot();

    const auto dataset =
        buildCompositionDataset(
            {
                {"a", snapshot},
                {"b", snapshot},
                {"c", snapshot}
            });

    const auto partition =
        buildCompositionDatasetPartition(
            dataset,
            0.20,
            0.20);

    expect(
        partition.trainingCount() == 1,
        "small dataset keeps training split populated");

    expect(
        partition.validationCount() == 1,
        "small dataset keeps validation split populated");

    expect(
        partition.testCount() == 1,
        "small dataset keeps test split populated");
}

void testZeroRatioDisablesSplit()
{
    const auto dataset =
        buildDataset();

    const auto partition =
        buildCompositionDatasetPartition(
            dataset,
            0.0,
            0.20);

    expect(
        partition.validationCount() == 0,
        "zero validation ratio disables validation split");

    expect(
        partition.testCount() == 8,
        "test split follows requested ratio");

    expect(
        partition.trainingCount() == 32,
        "remaining samples go to training");
}

} // namespace

int main()
{
    testPartitionCoversDatasetExactlyOnce();
    testPartitionIsDeterministic();
    testPartitionIsIndependentOfInputConstructionOrder();
    testInvalidRatiosAreRejected();
    testInvalidDatasetIsRejected();
    testPartitionRatiosProduceExpectedCounts();
    testSmallDatasetKeepsRequestedSplitsPopulated();
    testZeroRatioDisablesSplit();

    std::cout
        << "MIDI-GenGX Composition Dataset Partition tests passed.\n";

    return 0;
}
