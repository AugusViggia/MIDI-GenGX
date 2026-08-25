#include "Music/CompositionNeuralArtifactTrainingService.h"
#include "Music/MotifDevelopment.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

using namespace midigengx::music;
using namespace midigengx::domain;

namespace
{

void expect(
    bool condition,
    const char* message)
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
        planPhraseStructure(
            context);

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

    Motif motif;
    motif.lengthBeats = 2.0;
    motif.notes =
    {
        MotifNote{0.0, 0.5, 0, 0},
        MotifNote{0.5, 0.5, 2, 1},
        MotifNote{1.0, 0.5, 5, -1},
        MotifNote{1.5, 0.5, 3, 0}
    };

    const auto motifGraph =
        buildMotifOccurrenceGraph(
            {
                motif,
                MotifDevelopment::transpose(
                    motif,
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
         index < 32;
         ++index)
    {
        inputs.push_back(
        {
            "artifact-training-" +
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

CompositionNeuralTrainingConfig buildConfig()
{
    CompositionNeuralTrainingConfig config;

    config.epochs = 24;
    config.learningRate = 0.001;
    config.optimizer =
        CompositionNeuralOptimizer::Adam;
    config.gradientClip = 5.0;

    return config;
}

void testTrainingProducesArtifact()
{
    const auto prepared =
        buildPrepared();

    const auto result =
        trainCompositionNeuralArtifact(
            prepared,
            buildConfig());

    expect(
        result.isValid(),
        "training produces a valid neural artifact");

    expect(
        result.training.trained,
        "training result reports trained model");

    expect(
        result.training.epochsCompleted == 24,
        "training completes configured epochs");

    expect(
        result.artifact.isValid(),
        "trained artifact is valid");
}

void testTrainingActuallyChangesTheModel()
{
    const auto prepared =
        buildPrepared();

    const auto result =
        trainCompositionNeuralArtifact(
            prepared,
            buildConfig());

    expect(
        result.isValid(),
        "training result is valid before parameter comparison");

    expect(
        std::fabs(
            result.training.finalLoss -
            result.training.initialLoss) >
            1.0e-12,
        "training changes loss from its initial value");

    bool changed =
        false;

    for (const auto value :
         result.model.outputWeights)
    {
        if (std::fabs(value) > 0.0)
        {
            changed = true;
            break;
        }
    }

    expect(
        changed,
        "trained model contains learned output parameters");
}

void testTrainingImprovesOrMaintainsLoss()
{
    const auto prepared =
        buildPrepared();

    const auto result =
        trainCompositionNeuralArtifact(
            prepared,
            buildConfig());

    expect(
        result.isValid(),
        "training result is valid before loss assertion");

    expect(
        result.training.finalLoss <=
            result.training.initialLoss,
        "training does not increase final loss");
}

void testArtifactLoadsBackExactly()
{
    const auto prepared =
        buildPrepared();

    const auto result =
        trainCompositionNeuralArtifact(
            prepared,
            buildConfig());

    expect(
        result.isValid(),
        "trained artifact exists before round-trip");

    CompositionNeuralModel restored;

    expect(
        deserializeCompositionNeuralModel(
            result.artifact,
            restored),
        "trained artifact reloads");

    expect(
        restored.isValid(),
        "reloaded trained model is valid");

    expect(
        restored.inputWeights ==
            result.model.inputWeights &&
        restored.outputWeights ==
            result.model.outputWeights &&
        restored.outputBias ==
            result.model.outputBias &&
        restored.contextResidualWeights ==
            result.model.contextResidualWeights,
        "reloaded artifact preserves trained parameters");
}

void testTrainingIsDeterministic()
{
    const auto prepared =
        buildPrepared();

    const auto first =
        trainCompositionNeuralArtifact(
            prepared,
            buildConfig());

    const auto second =
        trainCompositionNeuralArtifact(
            prepared,
            buildConfig());

    expect(
        first.isValid() &&
        second.isValid(),
        "deterministic training runs are valid");

    expect(
        first.artifact.bytes ==
            second.artifact.bytes,
        "identical training runs produce identical artifacts");

    expect(
        first.training.initialLoss ==
            second.training.initialLoss &&
        first.training.finalLoss ==
            second.training.finalLoss,
        "identical training runs produce identical losses");
}

void testInvalidPreparedDatasetIsRejected()
{
    CompositionDatasetPreparedView invalid;

    const auto result =
        trainCompositionNeuralArtifact(
            invalid,
            buildConfig());

    expect(
        !result.valid,
        "invalid prepared dataset is rejected");
}

} // namespace

int main()
{
    testTrainingProducesArtifact();
    testTrainingActuallyChangesTheModel();
    testTrainingImprovesOrMaintainsLoss();
    testArtifactLoadsBackExactly();
    testTrainingIsDeterministic();
    testInvalidPreparedDatasetIsRejected();

    std::cout
        << "MIDI-GenGX Composition Neural Artifact Training Service tests passed.\n";

    return 0;
}
