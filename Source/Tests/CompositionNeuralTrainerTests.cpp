#include "Music/CompositionNeuralTrainer.h"
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
         index < 20;
         ++index)
    {
        inputs.push_back(
        {
            "trainer-" +
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

void testTrainingProducesValidResult()
{
    const auto prepared =
        buildPrepared();

    const auto contract =
        buildContract(
            prepared);

    auto model =
        initializeCompositionNeuralModel(
            contract);

    CompositionNeuralTrainingConfig config;
    config.epochs = 10;
    config.learningRate = 0.001;
    config.optimizer =
        CompositionNeuralOptimizer::Adam;

    const auto result =
        trainCompositionNeuralModel(
            model,
            prepared,
            config);

    expect(
        result.isValid(),
        "neural training result is valid");

    expect(
        result.epochsCompleted == 10,
        "all requested epochs completed");

    expect(
        result.finalLoss >= 0.0 &&
        result.initialLoss >= 0.0,
        "training losses are non-negative");
}

void testTrainingChangesModelParameters()
{
    const auto prepared =
        buildPrepared();

    const auto contract =
        buildContract(
            prepared);

    auto model =
        initializeCompositionNeuralModel(
            contract);

    const auto beforeInput =
        model.inputWeights;
    const auto beforeHidden =
        model.hiddenBias;
    const auto beforeOutput =
        model.outputWeights;
    const auto beforeBias =
        model.outputBias;
    const auto beforeResidual =
        model.contextResidualWeights;

    CompositionNeuralTrainingConfig config;
    config.epochs = 3;
    config.learningRate = 0.01;
    config.optimizer =
        CompositionNeuralOptimizer::Adam;

    const auto result =
        trainCompositionNeuralModel(
            model,
            prepared,
            config);

    expect(
        result.isValid(),
        "training completed");

    expect(
        beforeInput !=
            model.inputWeights,
        "backpropagation changes input weights");

    expect(
        beforeHidden !=
            model.hiddenBias,
        "backpropagation changes hidden biases");

    expect(
        beforeOutput !=
            model.outputWeights,
        "backpropagation changes output weights");

    expect(
        beforeBias !=
            model.outputBias,
        "backpropagation changes output biases");

    expect(
        beforeResidual !=
            model.contextResidualWeights,
        "backpropagation changes context residual weights");
}

void testTrainingIsDeterministic()
{
    const auto prepared =
        buildPrepared();

    const auto contract =
        buildContract(
            prepared);

    auto first =
        initializeCompositionNeuralModel(
            contract);

    auto second =
        initializeCompositionNeuralModel(
            contract);

    CompositionNeuralTrainingConfig config;
    config.epochs = 5;
    config.learningRate = 0.001;
    config.optimizer =
        CompositionNeuralOptimizer::Adam;

    const auto firstResult =
        trainCompositionNeuralModel(
            first,
            prepared,
            config);

    const auto secondResult =
        trainCompositionNeuralModel(
            second,
            prepared,
            config);

    expect(
        firstResult.finalLoss ==
            secondResult.finalLoss,
        "deterministic training produces identical loss");

    expect(
        first.outputBias ==
            second.outputBias &&
        first.contextResidualWeights ==
            second.contextResidualWeights,
        "deterministic training produces identical output and residual parameters");
}

void testSgdOptimizerPath()
{
    const auto prepared =
        buildPrepared();

    const auto contract =
        buildContract(
            prepared);

    auto model =
        initializeCompositionNeuralModel(
            contract);

    CompositionNeuralTrainingConfig config;
    config.epochs = 5;
    config.learningRate = 0.005;
    config.optimizer =
        CompositionNeuralOptimizer::SGD;
    config.gradientClip = 1.0;

    const auto result =
        trainCompositionNeuralModel(
            model,
            prepared,
            config);

    expect(
        result.isValid(),
        "SGD optimizer path is valid");
}

void testAdamIsDeterministic()
{
    const auto prepared =
        buildPrepared();

    const auto contract =
        buildContract(
            prepared);

    auto first =
        initializeCompositionNeuralModel(
            contract);

    auto second =
        initializeCompositionNeuralModel(
            contract);

    CompositionNeuralTrainingConfig config;
    config.epochs = 8;
    config.learningRate = 0.001;
    config.optimizer =
        CompositionNeuralOptimizer::Adam;

    const auto firstResult =
        trainCompositionNeuralModel(
            first,
            prepared,
            config);

    const auto secondResult =
        trainCompositionNeuralModel(
            second,
            prepared,
            config);

    expect(
        firstResult.finalLoss ==
            secondResult.finalLoss,
        "Adam training is deterministic");

    expect(
        first.inputWeights ==
            second.inputWeights &&
        first.outputWeights ==
            second.outputWeights &&
        first.hiddenBias ==
            second.hiddenBias &&
        first.outputBias ==
            second.outputBias,
        "Adam parameters are deterministic");
}

void testLossIsStableOrImproves()
{
    const auto prepared =
        buildPrepared();

    const auto contract =
        buildContract(
            prepared);

    auto model =
        initializeCompositionNeuralModel(
            contract);

    CompositionNeuralTrainingConfig config;
    config.epochs = 20;
    config.learningRate = 0.01;
    config.gradientClip = 1.0;

    const auto result =
        trainCompositionNeuralModel(
            model,
            prepared,
            config);

    expect(
        result.isValid(),
        "longer backpropagation training completes");

    expect(
        result.finalLoss <=
            result.initialLoss ||
        std::abs(
            result.finalLoss -
            result.initialLoss) <
            1.0e-12,
        "training loss does not diverge");
}

void testValidationAndTestDataAreNotUsed()
{
    const auto prepared =
        buildPrepared();

    const auto contract =
        buildContract(
            prepared);

    auto baseline =
        initializeCompositionNeuralModel(
            contract);

    auto modified =
        baseline;

    auto altered =
        prepared;

    for (const auto sampleIndex :
         altered.partition.validationIndices)
    {
        const auto base =
            sampleIndex *
            altered.normalizedBatch
                .globalFeatureWidth;

        altered.normalizedBatch
            .globalMatrix[base] =
                0.999999;
    }

    for (const auto sampleIndex :
         altered.partition.testIndices)
    {
        const auto base =
            sampleIndex *
            altered.normalizedBatch
                .globalFeatureWidth;

        altered.normalizedBatch
            .globalMatrix[base + 1] =
                0.000001;
    }

    CompositionNeuralTrainingConfig config;
    config.epochs = 5;
    config.learningRate = 0.01;

    trainCompositionNeuralModel(
        baseline,
        prepared,
        config);

    trainCompositionNeuralModel(
        modified,
        altered,
        config);

    expect(
        baseline.outputBias ==
            modified.outputBias,
        "validation/test changes do not affect training");
}

void testInvalidConfigurationIsRejected()
{
    const auto prepared =
        buildPrepared();

    const auto contract =
        buildContract(
            prepared);

    auto model =
        initializeCompositionNeuralModel(
            contract);

    CompositionNeuralTrainingConfig zeroEpochs;
    zeroEpochs.epochs = 0;

    expect(
        !trainCompositionNeuralModel(
             model,
             prepared,
             zeroEpochs)
             .trained,
        "zero epochs are rejected");

    CompositionNeuralTrainingConfig invalidRate;
    invalidRate.epochs = 5;
    invalidRate.learningRate = -0.01;

    expect(
        !trainCompositionNeuralModel(
             model,
             prepared,
             invalidRate)
             .trained,
        "negative learning rate is rejected");

    CompositionNeuralTrainingConfig invalidClip;
    invalidClip.epochs = 5;
    invalidClip.learningRate = 0.01;
    invalidClip.gradientClip = 0.0;

    expect(
        !trainCompositionNeuralModel(
             model,
             prepared,
             invalidClip)
             .trained,
        "non-positive gradient clip is rejected");

    CompositionNeuralTrainingConfig invalidAdam;
    invalidAdam.epochs = 5;
    invalidAdam.learningRate = 0.001;
    invalidAdam.optimizer =
        CompositionNeuralOptimizer::Adam;
    invalidAdam.beta1 = 1.0;

    expect(
        !trainCompositionNeuralModel(
             model,
             prepared,
             invalidAdam)
             .trained,
        "invalid Adam beta is rejected");
}

void testEmptyPreparedDataIsRejected()
{
    const auto validPrepared =
        buildPrepared();

    const auto contract =
        buildContract(
            validPrepared);

    auto neuralModel =
        initializeCompositionNeuralModel(
            contract);

    CompositionDatasetPreparedView empty;

    const auto result =
        trainCompositionNeuralModel(
            neuralModel,
            empty,
            CompositionNeuralTrainingConfig{});

    expect(
        neuralModel.isValid(),
        "valid neural model is available for empty-data rejection test");

    expect(
        !result.trained,
        "empty prepared data cannot train");
}

} // namespace

int main()
{
    testTrainingProducesValidResult();
    testTrainingChangesModelParameters();
    testTrainingIsDeterministic();
    testSgdOptimizerPath();
    testAdamIsDeterministic();
    testLossIsStableOrImproves();
    testValidationAndTestDataAreNotUsed();
    testInvalidConfigurationIsRejected();
    testEmptyPreparedDataIsRejected();

    std::cout
        << "MIDI-GenGX Composition Neural Trainer tests passed.\n";

    return 0;
}
