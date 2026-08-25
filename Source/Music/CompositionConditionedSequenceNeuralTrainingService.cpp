#include "CompositionConditionedSequenceNeuralTrainingService.h"

namespace midigengx::music
{

bool CompositionConditionedSequenceNeuralTrainingServiceResult::isValid()
    const noexcept
{
    return valid &&
           training.isValid() &&
           artifact.isValid();
}

CompositionConditionedSequenceNeuralTrainingServiceResult
trainCompositionConditionedSequenceNeuralModelFromDataset(
    const CompositionConditionedTrainingDataset& dataset,
    const CompositionConditionedSequenceNeuralTrainingConfig& config)
    noexcept
{
    CompositionConditionedSequenceNeuralTrainingServiceResult result;

    if (!dataset.isValid())
        return result;

    const auto contract =
        buildCompositionSequenceLearningContract(
            64,
            CompositionSequenceLearningObjective::NextEventPrediction);

    if (!contract.isValid())
        return result;

    auto model =
        initializeCompositionConditionedSequenceNeuralModel(
            contract,
            dataset.vocabulary);

    if (!model.isValid())
        return result;

    result.training =
        trainCompositionConditionedSequenceNeuralModel(
            model,
            dataset,
            config);

    if (!result.training.isValid())
        return result;

    result.artifact =
        serializeCompositionConditionedSequenceNeuralModel(
            model);

    if (!result.artifact.isValid())
        return result;

    result.valid =
        true;

    return result;
}

} // namespace midigengx::music
