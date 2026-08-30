#pragma once

#include "CompositionConditionedTrainingDataset.h"
#include "CompositionConditionedSequenceNeuralModel.h"

#include <cstddef>
#include <functional>

namespace midigengx::music
{

struct CompositionConditionedSequenceNeuralTrainingConfig
{
    std::size_t epochs = 25;
    double learningRate = 0.001;
    double gradientClip = 1.0;

    // 0 = automatic hardware-concurrency selection.
    std::size_t workerThreads = 0;

    // Full loss evaluation every N epochs.
    std::size_t evaluationInterval = 1;

    // Optional (epoch, totalEpochs, epochLoss) callback.
    std::function<void(
        std::size_t,
        std::size_t,
        double)> progressCallback;
};

struct CompositionConditionedSequenceNeuralTrainingResult
{
    bool trained = false;
    std::size_t epochsCompleted = 0;
    std::size_t windowCount = 0;
    double initialLoss = 0.0;
    double finalLoss = 0.0;

    bool isValid() const noexcept;
};

CompositionConditionedSequenceNeuralTrainingResult
trainCompositionConditionedSequenceNeuralModel(
    CompositionConditionedSequenceNeuralModel& model,
    const CompositionConditionedTrainingDataset& dataset,
    const CompositionConditionedSequenceNeuralTrainingConfig& config) noexcept;

} // namespace midigengx::music
