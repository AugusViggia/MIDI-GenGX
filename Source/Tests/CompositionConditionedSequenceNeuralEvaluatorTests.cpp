#include "Music/CompositionConditionedSequenceNeuralEvaluator.h"

#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <string>

using namespace midigengx::music;

namespace
{

void expect(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}

CompositionMidiTrainingSequence makeSequence(
    const std::string& id)
{
    CompositionMidiTrainingSequence sequence;
    sequence.sampleId = id;

    for (std::size_t index = 0;
         index < 80;
         ++index)
    {
        CompositionMidiTrainingEvent event;
        event.features =
        {
            0.0,
            60.0 / 127.0,
            0.5,
            0.75,
            0.0,
            0.25,
            0.5,
            0.25,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0
        };
        sequence.events.push_back(event);
    }

    sequence.featureWidth =
        CompositionMidiTrainingEvent::featureCount;

    sequence.analysisValid =
        true;
    return sequence;
}

CompositionMidiTrainingSequence makeZeroSequence(
    const std::string& id)
{
    auto sequence = makeSequence(id);
    for (auto& event : sequence.events)
        std::fill(event.features.begin(), event.features.end(), 0.0);
    return sequence;
}

CompositionSequenceMetadata makeMetadata(
    const std::string& id)
{
    CompositionSequenceMetadata metadata;
    metadata.sampleId = id;
    metadata.composerId = "chopin";
    metadata.workId = id + "_work";
    metadata.movementId = "single";
    metadata.styleId = "romantic";
    metadata.eraId = "romantic";
    metadata.instrumentationId = "solo_piano";
    metadata.verified = true;
    metadata.valid = true;
    return metadata;
}

void testEvaluatorProducesFiniteLoss()
{
    const auto metadata =
        buildCompositionSequenceMetadataCatalog(
        {
            makeMetadata("a"),
            makeMetadata("b")
        });

    const auto dataset =
        buildCompositionConditionedTrainingDataset(
        {
            makeSequence("a"),
            makeSequence("b")
        },
        metadata);

    expect(dataset.isValid(), "evaluation fixture dataset is valid");

    const auto contract =
        buildCompositionSequenceLearningContract(
            64,
            CompositionSequenceLearningObjective::NextEventPrediction);

    const auto model =
        initializeCompositionConditionedSequenceNeuralModel(
            contract,
            dataset.vocabulary);

    expect(model.isValid(), "evaluation fixture model is valid");

    const auto evaluation =
        evaluateCompositionConditionedSequenceNeuralModel(
            model,
            dataset);

    expect(evaluation.isValid(), "evaluation produces a valid result");
    expect(evaluation.sampleCount == 2 &&
           evaluation.windowCount > 0 &&
           evaluation.loss >= 0.0,
           "evaluation reports counts and non-negative loss");

    const auto weightedMetadata =
        buildCompositionSequenceMetadataCatalog(
        {
            makeMetadata("weighted-a"),
            makeMetadata("weighted-b")
        });

    const auto weightedDataset =
        buildCompositionConditionedTrainingDataset(
        {
            makeZeroSequence("weighted-a"),
            makeZeroSequence("weighted-b")
        },
        weightedMetadata);

    expect(weightedDataset.isValid(),
           "weighted evaluation fixture dataset is valid");

    const auto weightedContract =
        buildCompositionSequenceLearningContract(
            64,
            CompositionSequenceLearningObjective::NextEventPrediction);

    const auto weightedBaseModel =
        initializeCompositionConditionedSequenceNeuralModel(
            weightedContract,
            weightedDataset.vocabulary);

    expect(weightedBaseModel.isValid(),
           "weighted evaluation fixture model is valid");

    auto weightedModel = weightedBaseModel;
    std::fill(weightedModel.inputWeights.begin(), weightedModel.inputWeights.end(), 0.0);
    std::fill(weightedModel.recurrentWeights.begin(), weightedModel.recurrentWeights.end(), 0.0);
    std::fill(weightedModel.hiddenBias.begin(), weightedModel.hiddenBias.end(), 0.0);
    std::fill(weightedModel.outputWeights.begin(), weightedModel.outputWeights.end(), 0.0);
    std::fill(weightedModel.outputBias.begin(), weightedModel.outputBias.end(), 0.0);
    std::fill(weightedModel.composerEmbeddings.begin(), weightedModel.composerEmbeddings.end(), 0.0);
    std::fill(weightedModel.styleEmbeddings.begin(), weightedModel.styleEmbeddings.end(), 0.0);
    std::fill(weightedModel.eraEmbeddings.begin(), weightedModel.eraEmbeddings.end(), 0.0);
    std::fill(weightedModel.instrumentationEmbeddings.begin(), weightedModel.instrumentationEmbeddings.end(), 0.0);

    weightedModel.outputBias[0] = 1.0;
    weightedModel.outputBias[4] = 1.0;

    const auto weightedEvaluation =
        evaluateCompositionConditionedSequenceNeuralModel(
            weightedModel,
            weightedDataset);

    constexpr double expectedWeightedLoss = 3.25 / 12.0;
    expect(weightedEvaluation.isValid(),
           "weighted evaluation produces a valid result");
    expect(std::abs(weightedEvaluation.loss - expectedWeightedLoss) < 1.0e-9,
           "evaluation applies structured loss weights");
}

void testInvalidDatasetFailsClosed()
{
    CompositionConditionedTrainingDataset invalid;

    CompositionConditioningVocabulary vocabulary;
    vocabulary.composers = {"chopin"};
    vocabulary.styles = {"romantic"};
    vocabulary.eras = {"romantic"};
    vocabulary.instrumentations = {"solo_piano"};
    vocabulary.valid = true;

    const auto contract =
        buildCompositionSequenceLearningContract(
            64,
            CompositionSequenceLearningObjective::NextEventPrediction);

    const auto model =
        initializeCompositionConditionedSequenceNeuralModel(
            contract,
            vocabulary);

    const auto evaluation =
        evaluateCompositionConditionedSequenceNeuralModel(
            model,
            invalid);

    expect(!evaluation.isValid(), "invalid dataset fails evaluation");
}

} // namespace

int main()
{
    testEvaluatorProducesFiniteLoss();
    testInvalidDatasetFailsClosed();

    std::cout
        << "MIDI-GenGX Phase 107 neural evaluator tests passed.\n";

    return 0;
}
