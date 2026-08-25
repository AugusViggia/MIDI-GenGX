#include "Music/CompositionRuntimeInferenceService.h"
#include "Music/MotifDevelopment.h"
#include "Music/CompositionNeuralTrainer.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

using namespace midigengx::music;
using namespace midigengx::domain;

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

CompositionDatasetPreparedView buildPrepared()
{
    MusicalContext context;
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

    Motif motifA;
    motifA.lengthBeats = 2.0;
    motifA.notes =
    {
        MotifNote{0.0, 0.5, 0, 0},
        MotifNote{0.5, 0.5, 2, 1},
        MotifNote{1.0, 0.5, 5, -1},
        MotifNote{1.5, 0.5, 3, 0}
    };

    const auto motifGraph =
        buildMotifOccurrenceGraph(
            {
                motifA,
                MotifDevelopment::transpose(
                    motifA,
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
         index < 24;
         ++index)
    {
        inputs.push_back(
        {
            "runtime-service-" +
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

CompositionInferencePipeline buildPipeline()
{
    const auto prepared =
        buildPrepared();

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

    const auto training =
        trainCompositionNeuralModel(
            model,
            prepared,
            config);

    expect(
        training.isValid(),
        "runtime inference service model training is valid");

    return buildCompositionInferencePipeline(
        model);
}

MusicalContext buildContext()
{
    MusicalContext context;
    context.key = Key::G;
    context.scale = Scale{
        ScaleType::Minor};
    context.role = Role::Lead;
    context.parameters.lengthBars = 8;
    context.parameters.phraseLengthBars = 4;
    context.parameters.tension = 65;
    context.parameters.complexity = 55;
    context.normalize();
    return context;
}

void testServiceBuildsFromPipeline()
{
    const auto pipeline =
        buildPipeline();

    const CompositionRuntimeInferenceService service(
        pipeline);

    expect(
        service.isValid(),
        "runtime inference service is valid");
}

void testMusicalContextReachesNeuralInference()
{
    const auto pipeline =
        buildPipeline();

    const CompositionRuntimeInferenceService service(
        pipeline);

    const auto result =
        service.infer(
            buildContext());

    expect(
        result.isValid(),
        "runtime service produces valid neural inference");

    expect(
        result.request.contextIsValid,
        "runtime request reaches inference as valid");

    expect(
        result.request.globalFeatures.size() ==
            pipeline.contract.globalInputWidth,
        "runtime global features match contract");

    expect(
        result.request.contextSectionFeatures.size() ==
            pipeline.contract.sectionInputWidth,
        "runtime section features match contract");

    expect(
        result.inference.prediction.sectionFeatures.size() ==
            pipeline.contract.targetWidth,
        "neural output width matches contract");
}

void testContextChangesReachInference()
{
    const auto pipeline =
        buildPipeline();

    const CompositionRuntimeInferenceService service(
        pipeline);

    auto lowTension =
        buildContext();

    auto highTension =
        lowTension;

    lowTension.parameters.tension = 10;
    highTension.parameters.tension = 90;

    const auto low =
        service.infer(
            lowTension);

    const auto high =
        service.infer(
            highTension);

    expect(
        low.isValid() &&
        high.isValid(),
        "changed contexts both infer successfully");

    const auto tensionIndex =
        CompositionDatasetSchema::sectionFeatureIndex(
            SectionFeature::TensionNormalized);

    expect(
        low.request.contextSectionFeatures[
            tensionIndex] <
            high.request.contextSectionFeatures[
                tensionIndex],
        "context tension reaches runtime request");
}

void testInferenceIsDeterministic()
{
    const auto pipeline =
        buildPipeline();

    const CompositionRuntimeInferenceService service(
        pipeline);

    const auto context =
        buildContext();

    const auto first =
        service.infer(
            context);

    const auto second =
        service.infer(
            context);

    expect(
        first.isValid() &&
        second.isValid(),
        "determinism inference results are valid");

    expect(
        first.inference.prediction.sectionFeatures ==
            second.inference.prediction.sectionFeatures,
        "runtime inference is deterministic");
}

void testInvalidPipelineIsRejected()
{
    CompositionRuntimeInferenceService service;

    expect(
        !service.isValid(),
        "empty runtime inference service is invalid");

    const auto result =
        service.infer(
            buildContext());

    expect(
        !result.valid,
        "invalid runtime inference service does not infer");
}

} // namespace

int main()
{
    testServiceBuildsFromPipeline();
    testMusicalContextReachesNeuralInference();
    testContextChangesReachInference();
    testInferenceIsDeterministic();
    testInvalidPipelineIsRejected();

    std::cout
        << "MIDI-GenGX Composition Runtime Inference Service tests passed.\n";

    return 0;
}
