#pragma once

#include "CompositionSequenceNeuralModel.h"

#include <cstddef>
#include <vector>

namespace midigengx::music
{

struct CompositionSequenceNeuralTrainingConfig
{
    std::size_t epochs = 25;
    double learningRate = 0.001;
    double gradientClip = 1.0;
};

struct CompositionSequenceNeuralTrainingResult
{
    bool trained = false;
    std::size_t epochsCompleted = 0;
    std::size_t windowCount = 0;
    double initialLoss = 0.0;
    double finalLoss = 0.0;

    bool isValid() const noexcept;
};

CompositionSequenceNeuralTrainingResult
trainCompositionSequenceNeuralModel(
    CompositionSequenceNeuralModel& model,
    const std::vector<CompositionMidiTrainingSequence>& sequences,
    const CompositionSequenceNeuralTrainingConfig& config) noexcept;

} // namespace midigengx::music
