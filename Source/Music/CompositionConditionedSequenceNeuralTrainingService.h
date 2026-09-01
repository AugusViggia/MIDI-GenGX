#pragma once

#include "CompositionConditionedSequenceNeuralModelArtifact.h"
#include "CompositionConditionedSequenceNeuralTrainer.h"
#include "CompositionKnowledgeTrainingDataset.h"

namespace midigengx::music
{

struct CompositionConditionedSequenceNeuralTrainingServiceResult
{
    CompositionConditionedSequenceNeuralTrainingResult training;
    CompositionConditionedSequenceNeuralModelArtifact artifact;

    bool valid = false;

    bool isValid() const noexcept;
};

CompositionConditionedSequenceNeuralTrainingServiceResult
trainCompositionConditionedSequenceNeuralModelFromDataset(
    const CompositionConditionedTrainingDataset& dataset,
    const CompositionConditionedSequenceNeuralTrainingConfig& config) noexcept;

CompositionConditionedSequenceNeuralTrainingServiceResult
trainCompositionConditionedSequenceNeuralModelFromDataset(
    const CompositionConditionedTrainingDataset& dataset,
    const CompositionKnowledgeTrainingDataset& knowledgeDataset,
    const CompositionConditionedSequenceNeuralTrainingConfig& config) noexcept;

} // namespace midigengx::music
