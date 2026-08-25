#pragma once

#include "CompositionConditionedSequenceNeuralModel.h"

namespace midigengx::music
{

struct CompositionConditionedSequenceNeuralEvaluationResult
{
    std::size_t sampleCount = 0;
    std::size_t windowCount = 0;
    double loss = 0.0;
    bool valid = false;

    bool isValid() const noexcept;
};

CompositionConditionedSequenceNeuralEvaluationResult
evaluateCompositionConditionedSequenceNeuralModel(
    const CompositionConditionedSequenceNeuralModel& model,
    const CompositionConditionedTrainingDataset& dataset) noexcept;

} // namespace midigengx::music
