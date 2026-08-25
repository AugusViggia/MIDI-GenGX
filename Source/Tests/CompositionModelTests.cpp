#include "Music/CompositionModel.h"
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

CompositionDatasetPreparedView buildPrepared(
    std::size_t sampleCount)
{
    const auto snapshot =
        buildSnapshot();

    std::vector<CompositionDatasetInput>
        inputs;

    for (std::size_t index = 0;
         index < sampleCount;
         ++index)
    {
        inputs.push_back(
        {
            "model-" +
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

CompositionLearningContract buildContract(
    const CompositionDatasetPreparedView& prepared)
{
    return buildCompositionLearningContract(
        prepared,
        LearningObjective::NextSectionPrediction);
}

void testTrainingProducesValidModel()
{
    const auto prepared =
        buildPrepared(20);

    const auto contract =
        buildContract(
            prepared);

    const auto model =
        trainCompositionBaselineModel(
            prepared,
            contract);

    expect(
        model.trained,
        "baseline model reports trained state");

    expect(
        model.isValid(),
        "trained baseline model is valid");

    expect(
        model.nextSectionMean.size() ==
            contract.targetWidth,
        "model target width matches contract");
}

void testPredictionShapeAndFiniteness()
{
    const auto prepared =
        buildPrepared(20);

    const auto contract =
        buildContract(
            prepared);

    const auto model =
        trainCompositionBaselineModel(
            prepared,
            contract);

    const auto& batch =
        prepared.normalizedBatch;

    const auto globalFeatures =
        std::vector<double>(
            batch.globalMatrix.begin(),
            batch.globalMatrix.begin() +
                batch.globalFeatureWidth);

    const auto contextBase =
        0 * batch.maxSectionCount *
            batch.sectionFeatureWidth;

    const auto contextFeatures =
        std::vector<double>(
            batch.sectionMatrix.begin() +
                contextBase,
            batch.sectionMatrix.begin() +
                contextBase +
                batch.sectionFeatureWidth);

    const auto prediction =
        model.predictNextSection(
            globalFeatures,
            contextFeatures,
            true);

    expect(
        prediction.isValid(
            contract.targetWidth),
        "baseline prediction is valid");

    for (const auto value :
         prediction.sectionFeatures)
    {
        expect(
            std::isfinite(value),
            "baseline prediction values are finite");
    }
}

void testValidationAndTestDataCannotTrainModel()
{
    const auto prepared =
        buildPrepared(20);

    const auto contract =
        buildContract(
            prepared);

    auto modified =
        prepared;

    for (const auto sampleIndex :
         modified.partition.validationIndices)
    {
        const auto base =
            sampleIndex *
            modified.normalizedBatch
                .globalFeatureWidth;

        modified.normalizedBatch
            .globalMatrix[base] =
                0.999999;
    }

    for (const auto sampleIndex :
         modified.partition.testIndices)
    {
        const auto base =
            sampleIndex *
            modified.normalizedBatch
                .globalFeatureWidth;

        modified.normalizedBatch
            .globalMatrix[base + 1] =
                0.000001;
    }

    const auto first =
        trainCompositionBaselineModel(
            prepared,
            contract);

    const auto second =
        trainCompositionBaselineModel(
            modified,
            contract);

    expect(
        first.nextSectionMean ==
            second.nextSectionMean,
        "validation/test changes do not alter trained parameters");
}

void testPredictionRejectsInvalidContext()
{
    const auto prepared =
        buildPrepared(20);

    const auto contract =
        buildContract(
            prepared);

    const auto model =
        trainCompositionBaselineModel(
            prepared,
            contract);

    const auto prediction =
        model.predictNextSection(
            {},
            {},
            false);

    expect(
        !prediction.valid,
        "invalid context is rejected");
}

void testUntrainedModelCannotPredict()
{
    CompositionModel model;

    const auto prediction =
        model.predictNextSection(
            std::vector<double>(
                CompositionDatasetSchema::globalFeatureCount,
                0.0),
            std::vector<double>(
                CompositionDatasetSchema::sectionFeatureCount,
                0.0),
            true);

    expect(
        !prediction.valid,
        "untrained model cannot produce prediction");
}

void testEmptyPreparedDataCannotTrainModel()
{
    const auto prepared =
        buildPrepared(0);

    const auto contract =
        buildContract(
            prepared);

    const auto model =
        trainCompositionBaselineModel(
            prepared,
            contract);

    expect(
        !model.trained,
        "empty prepared data does not train a model");

    expect(
        !model.isValid(),
        "empty-trained baseline remains invalid");
}

} // namespace

int main()
{
    testTrainingProducesValidModel();
    testPredictionShapeAndFiniteness();
    testValidationAndTestDataCannotTrainModel();
    testPredictionRejectsInvalidContext();
    testUntrainedModelCannotPredict();
    testEmptyPreparedDataCannotTrainModel();

    std::cout
        << "MIDI-GenGX Composition Model tests passed.\n";

    return 0;
}
