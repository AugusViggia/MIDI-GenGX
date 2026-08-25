#include "Music/CompositionAIGenerationCoordinator.h"
#include "Music/CompositionNeuralTrainer.h"
#include "Music/MotifDevelopment.h"

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

MusicalContext buildContext()
{
    MusicalContext context;

    context.key = Key::C;
    context.scale = Scale{ScaleType::Minor};
    context.role = Role::Lead;

    context.parameters.lengthBars = 8;
    context.parameters.phraseLengthBars = 4;
    context.parameters.octaveLow = 0;
    context.parameters.octaveHigh = 2;
    context.parameters.density = 55;
    context.parameters.tension = 40;
    context.parameters.variation = 45;
    context.parameters.complexity = 40;

    context.normalize();
    return context;
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
            "coordinator-" +
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

CompositionAIGenerationCoordinator
buildCoordinator()
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
        "neural model trains for coordinator tests");

    const auto pipeline =
        buildCompositionInferencePipeline(
            model);

    const auto bridge =
        buildCompositionAIEngineBridge(
            true);

    return buildCompositionAIGenerationCoordinator(
        pipeline,
        bridge,
        true);
}

CompositionAIGenerationRequest buildRequest()
{
    CompositionAIGenerationRequest request;

    request.context =
        buildContext();

    request.globalFeatures.assign(
        {
            0.25, 0.50, 0.20, 0.30, 0.10, 0.20,
            0.40, 0.20, 0.80, 0.30, 0.15, 0.10,
            0.70
        });

    request.contextSectionFeatures.assign(
        {
            0.00, 0.40, 0.10, 0.25, 0.20, 0.05
        });

    request.contextIsValid = true;
    request.seed = 777;

    return request;
}

void testCoordinatorIsValid()
{
    const auto coordinator =
        buildCoordinator();

    expect(
        coordinator.isValid(),
        "AI generation coordinator is valid");
}

void testCoordinatorUsesAI()
{
    const auto coordinator =
        buildCoordinator();

    const auto result =
        coordinator.generate(
            buildRequest());

    expect(
        result.isValid(),
        "AI coordinator produces a valid phrase");

    expect(
        result.usedAI,
        "AI coordinator actually uses the AI path");

    expect(
        result.guidance.isValid(),
        "AI guidance is present in the result");
}

void testCoordinatorIsDeterministic()
{
    const auto coordinator =
        buildCoordinator();

    const auto first =
        coordinator.generate(
            buildRequest());

    const auto second =
        coordinator.generate(
            buildRequest());

    expect(
        first.usedAI &&
        second.usedAI,
        "both generations use AI");

    expect(
        first.phrase.notes.size() ==
            second.phrase.notes.size(),
        "AI coordinator generation is deterministic");

    expect(
        first.phrase.lengthBeats ==
            second.phrase.lengthBeats,
        "AI coordinator phrase length is deterministic");
}

void testDisabledCoordinatorDoesNotUseAI()
{
    const auto enabled =
        buildCoordinator();

    const auto disabled =
        buildCompositionAIGenerationCoordinator(
            enabled.pipeline,
            enabled.bridge,
            false);

    auto request =
        buildRequest();

    request.globalFeatures.clear();
    request.contextSectionFeatures.clear();
    request.contextIsValid = false;

    const auto result =
        disabled.generate(
            request);

    expect(
        !result.usedAI,
        "disabled coordinator does not use AI");

    expect(
        result.valid,
        "disabled coordinator safely falls back");

    expect(
        result.phrase.isValid(),
        "disabled coordinator returns a valid baseline phrase");
}

void testEnabledCoordinatorRejectsInvalidAIRequest()
{
    const auto coordinator =
        buildCoordinator();

    auto invalid =
        buildRequest();

    invalid.globalFeatures.clear();
    invalid.contextSectionFeatures.clear();
    invalid.contextIsValid = false;

    const auto result =
        coordinator.generate(
            invalid);

    expect(
        !result.valid,
        "enabled coordinator rejects invalid AI request");
}

void testInvalidRequestIsRejected()
{
    const auto coordinator =
        buildCoordinator();

    CompositionAIGenerationRequest invalid;

    const auto result =
        coordinator.generate(
            invalid);

    expect(
        !result.valid,
        "invalid coordinator request is rejected");
}

void testExplicitContextRemainsValid()
{
    const auto coordinator =
        buildCoordinator();

    auto request =
        buildRequest();

    request.context.parameters.lengthBars = 12;
    request.context.parameters.phraseLengthBars = 3;
    request.context.parameters.octaveLow = -1;
    request.context.parameters.octaveHigh = 1;
    request.context.normalize();

    const auto result =
        coordinator.generate(
            request);

    expect(
        result.isValid(),
        "AI coordinator preserves valid explicit context");

    for (const auto& note :
         result.phrase.notes)
    {
        expect(
            note.midiNote >= 0 &&
            note.midiNote <= 127,
            "coordinator preserves valid MIDI range");
    }
}

} // namespace

int main()
{
    testCoordinatorIsValid();
    testCoordinatorUsesAI();
    testCoordinatorIsDeterministic();
    testDisabledCoordinatorDoesNotUseAI();
    testEnabledCoordinatorRejectsInvalidAIRequest();
    testInvalidRequestIsRejected();
    testExplicitContextRemainsValid();

    std::cout
        << "MIDI-GenGX AI Generation Coordinator tests passed.\n";

    return 0;
}
