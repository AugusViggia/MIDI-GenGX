#include "CompositionConditionedSequenceNeuralTrainingService.h"

#include <unordered_map>

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


CompositionConditionedSequenceNeuralTrainingServiceResult
trainCompositionConditionedSequenceNeuralModelFromDataset(
    const CompositionConditionedTrainingDataset& dataset,
    const CompositionKnowledgeTrainingDataset& knowledgeDataset,
    const CompositionConditionedSequenceNeuralTrainingConfig& config) noexcept
{
    CompositionConditionedSequenceNeuralTrainingServiceResult result;

    if (!dataset.isValid() ||
        !knowledgeDataset.isValid() ||
        knowledgeDataset.sampleCount() != dataset.sampleCount())
    {
        return result;
    }

    auto enrichedDataset = dataset;

    std::unordered_map<
        std::string,
        const CompositionKnowledgeTrainingSample*>
        knowledgeBySampleId;

    knowledgeBySampleId.reserve(
        knowledgeDataset.samples.size());

    for (const auto& knowledgeSample :
         knowledgeDataset.samples)
    {
        if (!knowledgeSample.isValid() ||
            !knowledgeBySampleId.emplace(
                knowledgeSample.sampleId,
                &knowledgeSample).second)
        {
            return result;
        }
    }

    for (auto& conditionedSample :
         enrichedDataset.samples)
    {
        const auto it =
            knowledgeBySampleId.find(
                conditionedSample.sequence.sampleId);

        if (it == knowledgeBySampleId.end() ||
            it->second == nullptr)
        {
            return result;
        }

        conditionedSample.knowledgeFeatures =
            it->second->conditioningFeatures;
    }

    if (!enrichedDataset.isValid())
        return result;

    return trainCompositionConditionedSequenceNeuralModelFromDataset(
        enrichedDataset,
        config);
}

} // namespace midigengx::music
