#pragma once

#include "CompositionConditionedSequenceNeuralModelArtifact.h"
#include "CompositionConditionedSequenceNeuralTrainer.h"

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

} // namespace midigengx::music
