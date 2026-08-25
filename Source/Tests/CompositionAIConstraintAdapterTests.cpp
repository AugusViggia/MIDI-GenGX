#include "Music/CompositionAIConstraintAdapter.h"
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
            "ai-constraint-" +
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

CompositionInferencePipeline buildPipeline(
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

    const auto training =
        trainCompositionNeuralModel(
            model,
            prepared,
            config);

    expect(
        training.isValid(),
        "neural model trains for constraint tests");

    return buildCompositionInferencePipeline(
        model);
}

CompositionAIConstraintRequest buildRequest(
    const CompositionDatasetPreparedView& prepared)
{
    const auto& batch =
        prepared.normalizedBatch;

    CompositionAIConstraintRequest request;

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

void testAdapterBuildsValidConstraintProfile()
{
    const auto prepared =
        buildPrepared();

    const auto pipeline =
        buildPipeline(
            prepared);

    const auto request =
        buildRequest(
            prepared);

    const auto result =
        adaptAIResultToMusicalConstraints(
            pipeline,
            request);

    expect(
        result.isValid(),
        "AI result adapts to valid musical constraints");

    expect(
        result.profile.confidence == 1.0,
        "adapter confidence is explicitly defined");

    expect(
        result.profile.roleTarget >= -1.0 &&
        result.profile.roleTarget <= 1.0,
        "role target is normalized");
}

void testAllMusicalTargetsAreMapped()
{
    const auto prepared =
        buildPrepared();

    const auto pipeline =
        buildPipeline(
            prepared);

    const auto request =
        buildRequest(
            prepared);

    const auto inference =
        pipeline.infer(
            CompositionInferenceRequest{
                request.globalFeatures,
                request.contextSectionFeatures,
                request.contextIsValid
            });

    const auto result =
        adaptAIResultToMusicalConstraints(
            pipeline,
            request);

    expect(
        result.isValid(
        ),
        "constraint adaptation is valid");

    expect(
        result.profile.roleTarget ==
            inference.prediction.sectionFeatures[0] &&
        result.profile.tensionTarget ==
            inference.prediction.sectionFeatures[1] &&
        result.profile.tensionDeltaTarget ==
            inference.prediction.sectionFeatures[2] &&
        result.profile.harmonyDegreeTarget ==
            inference.prediction.sectionFeatures[3] &&
        result.profile.harmonyQualityTarget ==
            inference.prediction.sectionFeatures[4] &&
        result.profile.harmonicDegreeDeltaTarget ==
            inference.prediction.sectionFeatures[5],
        "all AI outputs map directly to musical constraint targets");
}

void testAdapterIsDeterministic()
{
    const auto prepared =
        buildPrepared();

    const auto pipeline =
        buildPipeline(
            prepared);

    const auto request =
        buildRequest(
            prepared);

    const auto first =
        adaptAIResultToMusicalConstraints(
            pipeline,
            request);

    const auto second =
        adaptAIResultToMusicalConstraints(
            pipeline,
            request);

    expect(
        first.profile.roleTarget ==
            second.profile.roleTarget &&
        first.profile.tensionTarget ==
            second.profile.tensionTarget &&
        first.profile.harmonyDegreeTarget ==
            second.profile.harmonyDegreeTarget,
        "AI-to-constraints adaptation is deterministic");
}

void testInvalidRequestIsRejected()
{
    const auto prepared =
        buildPrepared();

    const auto pipeline =
        buildPipeline(
            prepared);

    CompositionAIConstraintRequest invalid;

    const auto result =
        adaptAIResultToMusicalConstraints(
            pipeline,
            invalid);

    expect(
        !result.valid,
        "invalid constraint request is rejected");
}

void testInvalidPipelineIsRejected()
{
    CompositionInferencePipeline invalid;

    CompositionAIConstraintRequest request;
    request.globalFeatures.assign(
        13,
        0.0);
    request.contextSectionFeatures.assign(
        6,
        0.0);
    request.contextIsValid = true;

    const auto result =
        adaptAIResultToMusicalConstraints(
            invalid,
            request);

    expect(
        !result.valid,
        "invalid inference pipeline is rejected");
}

void testTargetsRemainInsideNormalizedRange()
{
    const auto prepared =
        buildPrepared();

    const auto pipeline =
        buildPipeline(
            prepared);

    const auto request =
        buildRequest(
            prepared);

    const auto result =
        adaptAIResultToMusicalConstraints(
            pipeline,
            request);

    expect(
        result.isValid(),
        "constraint profile is valid");

    const auto values = {
        result.profile.roleTarget,
        result.profile.tensionTarget,
        result.profile.tensionDeltaTarget,
        result.profile.harmonyDegreeTarget,
        result.profile.harmonyQualityTarget,
        result.profile.harmonicDegreeDeltaTarget
    };

    for (const auto value :
         values)
    {
        expect(
            value >= -1.0 &&
            value <= 1.0,
            "adapted musical target remains normalized");
    }
}

} // namespace

int main()
{
    testAdapterBuildsValidConstraintProfile();
    testAllMusicalTargetsAreMapped();
    testAdapterIsDeterministic();
    testInvalidRequestIsRejected();
    testInvalidPipelineIsRejected();
    testTargetsRemainInsideNormalizedRange();

    std::cout
        << "MIDI-GenGX AI Constraint Adapter tests passed.\n";

    return 0;
}
