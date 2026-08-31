
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

    // Structured weighting for the current 20-feature event representation.
    // Pitch and timing are the primary compositional targets; velocity is
    // secondary; auxiliary features remain lower-weighted until their
    // semantics are upgraded in the next output-head phase.
    double pitchLossWeight = 3.0;
    double timingLossWeight = 2.0;
    double velocityLossWeight = 1.0;
    double auxiliaryLossWeight = 0.25;

    // Returns the effective loss weight for the fixed-width event representation.
    // This is intentionally exposed for focused validation of the training contract.
    double lossWeightForFeature(std::size_t featureIndex) const noexcept;

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

