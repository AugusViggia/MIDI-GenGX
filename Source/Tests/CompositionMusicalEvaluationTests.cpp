#include "Music/CompositionMusicalEvaluation.h"
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
            "musical-" +
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

    trainCompositionNeuralModel(
        model,
        prepared,
        config);

    return model;
}

void testMusicalEvaluationIsValid()
{
    const auto prepared =
        buildPrepared();

    const auto model =
        buildTrainedModel(
            prepared);

    const auto result =
        evaluateCompositionNeuralMusicalQuality(
            model,
            prepared,
            true);

    expect(
        result.isValid(),
        "musical evaluation is valid");

    expect(
        result.exampleCount > 0,
        "musical evaluation contains examples");
}

void testScoresRemainNormalized()
{
    const auto prepared =
        buildPrepared();

    const auto model =
        buildTrainedModel(
            prepared);

    const auto result =
        evaluateCompositionNeuralMusicalQuality(
            model,
            prepared,
            false);

    expect(
        result.isValid(),
        "test musical evaluation is valid");

    expect(
        result.structuralCoherenceScore >= 0.0 &&
        result.structuralCoherenceScore <= 1.0,
        "structural coherence is normalized");

    expect(
        result.tensionConsistencyScore >= 0.0 &&
        result.tensionConsistencyScore <= 1.0,
        "tension consistency is normalized");

    expect(
        result.harmonicConsistencyScore >= 0.0 &&
        result.harmonicConsistencyScore <= 1.0,
        "harmonic consistency is normalized");

    expect(
        result.overallScore >= 0.0 &&
        result.overallScore <= 1.0,
        "overall musical score is normalized");
}

void testValidationAndTestRemainSeparated()
{
    const auto prepared =
        buildPrepared();

    const auto model =
        buildTrainedModel(
            prepared);

    const auto validation =
        evaluateCompositionNeuralMusicalQuality(
            model,
            prepared,
            true);

    const auto test =
        evaluateCompositionNeuralMusicalQuality(
            model,
            prepared,
            false);

    expect(
        validation.isValid() &&
        test.isValid(),
        "both musical evaluation splits are valid");

    expect(
        validation.exampleCount ==
            test.exampleCount,
        "the same section topology produces measurable split counts");
}

void testMusicalEvaluationIsDeterministic()
{
    const auto prepared =
        buildPrepared();

    const auto model =
        buildTrainedModel(
            prepared);

    const auto first =
        evaluateCompositionNeuralMusicalQuality(
            model,
            prepared,
            false);

    const auto second =
        evaluateCompositionNeuralMusicalQuality(
            model,
            prepared,
            false);

    expect(
        first.overallScore ==
            second.overallScore &&
        first.tensionConsistencyScore ==
            second.tensionConsistencyScore &&
        first.harmonicConsistencyScore ==
            second.harmonicConsistencyScore,
        "musical evaluation is deterministic");
}

void testInvalidModelIsRejected()
{
    const auto prepared =
        buildPrepared();

    CompositionNeuralModel invalid;

    const auto result =
        evaluateCompositionNeuralMusicalQuality(
            invalid,
            prepared,
            false);

    expect(
        !result.valid,
        "invalid neural model is rejected");
}

void testEmptyEvaluationSetIsRejected()
{
    const auto prepared =
        buildPrepared();

    const auto model =
        buildTrainedModel(
            prepared);

    auto empty =
        prepared;

    empty.partition.validationIndices.clear();

    const auto result =
        evaluateCompositionNeuralMusicalQuality(
            model,
            empty,
            true);

    expect(
        !result.valid,
        "empty musical evaluation split is rejected");
}

} // namespace

int main()
{
    testMusicalEvaluationIsValid();
    testScoresRemainNormalized();
    testValidationAndTestRemainSeparated();
    testMusicalEvaluationIsDeterministic();
    testInvalidModelIsRejected();
    testEmptyEvaluationSetIsRejected();

    std::cout
        << "MIDI-GenGX Composition Musical Evaluation tests passed.\n";

    return 0;
}
