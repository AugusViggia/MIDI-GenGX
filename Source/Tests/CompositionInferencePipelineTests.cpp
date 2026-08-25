#include "Music/CompositionInferencePipeline.h"
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
            "inference-" +
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

CompositionNeuralModel buildTrainedModel(
    const CompositionDatasetPreparedView& prepared)
{
    const auto contract =
        buildCompositionLearningContract(
            prepared,
            LearningObjective::NextSectionPrediction);

    auto model =
        initializeCompositionNeuralModel(
            contract);

    CompositionNeuralTrainingConfig config;
    config.epochs = 12;
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
        "neural model trains for inference tests");

    return model;
}

CompositionInferenceRequest buildRequest(
    const CompositionDatasetPreparedView& prepared)
{
    const auto& batch =
        prepared.normalizedBatch;

    CompositionInferenceRequest request;

    request.globalFeatures.assign(
        batch.globalMatrix.begin(),
        batch.globalMatrix.begin() +
            batch.globalFeatureWidth);

    request.contextSectionFeatures.assign(
        batch.sectionMatrix.begin(),
        batch.sectionMatrix.begin() +
            batch.sectionFeatureWidth);

    request.contextIsValid = true;

    return request;
}

void testPipelineBuildsFromTrainedModel()
{
    const auto prepared =
        buildPrepared();

    const auto model =
        buildTrainedModel(
            prepared);

    const auto pipeline =
        buildCompositionInferencePipeline(
            model);

    expect(
        pipeline.isValid(),
        "inference pipeline is valid");

    expect(
        pipeline.contract.objective ==
            LearningObjective::NextSectionPrediction,
        "inference pipeline preserves objective");
}

void testInferenceProducesValidPrediction()
{
    const auto prepared =
        buildPrepared();

    const auto model =
        buildTrainedModel(
            prepared);

    const auto pipeline =
        buildCompositionInferencePipeline(
            model);

    const auto request =
        buildRequest(
            prepared);

    const auto result =
        pipeline.infer(
            request);

    expect(
        result.isValid(
            pipeline.contract),
        "inference pipeline produces valid prediction");
}

void testInferenceIsDeterministic()
{
    const auto prepared =
        buildPrepared();

    const auto model =
        buildTrainedModel(
            prepared);

    const auto pipeline =
        buildCompositionInferencePipeline(
            model);

    const auto request =
        buildRequest(
            prepared);

    const auto first =
        pipeline.infer(
            request);

    const auto second =
        pipeline.infer(
            request);

    expect(
        first.prediction.sectionFeatures ==
            second.prediction.sectionFeatures,
        "inference pipeline is deterministic");
}

void testInvalidRequestIsRejected()
{
    const auto prepared =
        buildPrepared();

    const auto model =
        buildTrainedModel(
            prepared);

    const auto pipeline =
        buildCompositionInferencePipeline(
            model);

    CompositionInferenceRequest invalid;

    const auto result =
        pipeline.infer(
            invalid);

    expect(
        !result.valid,
        "invalid inference request is rejected");
}

void testDimensionMismatchIsRejected()
{
    const auto prepared =
        buildPrepared();

    const auto model =
        buildTrainedModel(
            prepared);

    const auto pipeline =
        buildCompositionInferencePipeline(
            model);

    auto request =
        buildRequest(
            prepared);

    request.globalFeatures.pop_back();

    expect(
        !pipeline.infer(
             request)
             .valid,
        "global feature width mismatch is rejected");

    request =
        buildRequest(
            prepared);

    request.contextSectionFeatures.pop_back();

    expect(
        !pipeline.infer(
             request)
             .valid,
        "context feature width mismatch is rejected");
}

void testInvalidModelDoesNotCreateReadyPipeline()
{
    CompositionNeuralModel invalid;

    const auto pipeline =
        buildCompositionInferencePipeline(
            invalid);

    expect(
        !pipeline.ready,
        "invalid model cannot create ready pipeline");

    expect(
        !pipeline.isValid(),
        "invalid model produces invalid inference pipeline");
}

} // namespace

int main()
{
    testPipelineBuildsFromTrainedModel();
    testInferenceProducesValidPrediction();
    testInferenceIsDeterministic();
    testInvalidRequestIsRejected();
    testDimensionMismatchIsRejected();
    testInvalidModelDoesNotCreateReadyPipeline();

    std::cout
        << "MIDI-GenGX Composition Inference Pipeline tests passed.\n";

    return 0;
}
