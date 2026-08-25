#include "Music/CompositionModelEvaluation.h"
#include "Music/CompositionNeuralTrainer.h"
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

CompositionDatasetPreparedView buildPrepared()
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

    const auto snapshot =
        buildCompositionKnowledgeSnapshot(
            composition,
            graph,
            transitions);

    std::vector<CompositionDatasetInput>
        inputs;

    for (int index = 0;
         index < 24;
         ++index)
    {
        inputs.push_back(
        {
            "eval-" +
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

void testBaselineValidationEvaluation()
{
    const auto prepared =
        buildPrepared();

    const auto baselinePreparedContract =
        buildContract(
            prepared);

    const auto baseline =
        trainCompositionBaselineModel(
            prepared,
            baselinePreparedContract);

    const auto evaluation =
        evaluateCompositionBaselineModel(
            baseline,
            prepared,
            true);

    expect(
        evaluation.isValid(),
        "baseline validation evaluation is valid");

    expect(
        evaluation.exampleCount > 0 &&
        evaluation.sampleCount ==
            prepared.validationCount(),
        "baseline validation coverage is correct");
}

void testNeuralTestEvaluation()
{
    const auto prepared =
        buildPrepared();

    const auto contract =
        buildContract(
            prepared);

    auto neural =
        initializeCompositionNeuralModel(
            contract);

    CompositionNeuralTrainingConfig config;
    config.epochs = 12;
    config.learningRate = 0.001;
    config.optimizer =
        CompositionNeuralOptimizer::Adam;

    const auto training =
        trainCompositionNeuralModel(
            neural,
            prepared,
            config);

    expect(
        training.isValid(),
        "neural model trains before evaluation");

    const auto evaluation =
        evaluateCompositionNeuralModel(
            neural,
            prepared,
            false);

    expect(
        evaluation.isValid(),
        "neural test evaluation is valid");

    expect(
        evaluation.exampleCount > 0 &&
        evaluation.sampleCount ==
            prepared.testCount(),
        "neural test coverage is correct");
}

void testValidationAndTestUseDifferentSplits()
{
    const auto prepared =
        buildPrepared();

    const auto contract =
        buildContract(
            prepared);

    const auto baseline =
        trainCompositionBaselineModel(
            prepared,
            contract);

    const auto validation =
        evaluateCompositionBaselineModel(
            baseline,
            prepared,
            true);

    const auto test =
        evaluateCompositionBaselineModel(
            baseline,
            prepared,
            false);

    expect(
        validation.isValid() &&
        test.isValid(),
        "both evaluation splits are valid");

    expect(
        validation.sampleCount ==
            prepared.validationCount() &&
        test.sampleCount ==
            prepared.testCount(),
        "validation/test sample counts remain separated");
}

void testEvaluationIsDeterministic()
{
    const auto prepared =
        buildPrepared();

    const auto contract =
        buildContract(
            prepared);

    auto neural =
        initializeCompositionNeuralModel(
            contract);

    CompositionNeuralTrainingConfig config;
    config.epochs = 8;
    config.learningRate = 0.001;
    config.optimizer =
        CompositionNeuralOptimizer::Adam;

    trainCompositionNeuralModel(
        neural,
        prepared,
        config);

    const auto first =
        evaluateCompositionNeuralModel(
            neural,
            prepared,
            false);

    const auto second =
        evaluateCompositionNeuralModel(
            neural,
            prepared,
            false);

    expect(
        first.meanSquaredError ==
            second.meanSquaredError &&
        first.meanAbsoluteError ==
            second.meanAbsoluteError &&
        first.exampleCount ==
            second.exampleCount,
        "evaluation metrics are deterministic");
}

void testInvalidModelIsRejected()
{
    const auto prepared =
        buildPrepared();

    CompositionModel invalidBaseline;

    const auto baselineResult =
        evaluateCompositionBaselineModel(
            invalidBaseline,
            prepared,
            true);

    expect(
        !baselineResult.valid,
        "invalid baseline model is rejected");

    CompositionNeuralModel invalidNeural;

    const auto neuralResult =
        evaluateCompositionNeuralModel(
            invalidNeural,
            prepared,
            false);

    expect(
        !neuralResult.valid,
        "invalid neural model is rejected");
}

void testEmptyEvaluationSetIsRejected()
{
    const auto prepared =
        buildPrepared();

    const auto contract =
        buildContract(
            prepared);

    const auto baseline =
        trainCompositionBaselineModel(
            prepared,
            contract);

    auto empty =
        prepared;

    empty.partition.validationIndices.clear();

    const auto result =
        evaluateCompositionBaselineModel(
            baseline,
            empty,
            true);

    expect(
        !result.valid,
        "empty evaluation split is rejected");
}

} // namespace

int main()
{
    testBaselineValidationEvaluation();
    testNeuralTestEvaluation();
    testValidationAndTestUseDifferentSplits();
    testEvaluationIsDeterministic();
    testInvalidModelIsRejected();
    testEmptyEvaluationSetIsRejected();

    std::cout
        << "MIDI-GenGX Composition Model Evaluation tests passed.\n";

    return 0;
}
