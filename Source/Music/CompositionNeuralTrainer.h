#pragma once

#include "CompositionNeuralModel.h"

#include <cstddef>

namespace midigengx::music
{

enum class CompositionNeuralOptimizer
{
    SGD,
    Adam
};

struct CompositionNeuralTrainingConfig
{
    std::size_t epochs = 100;
    double learningRate = 0.001;
    CompositionNeuralOptimizer optimizer =
        CompositionNeuralOptimizer::Adam;

    double beta1 = 0.9;
    double beta2 = 0.999;
    double epsilon = 1.0e-8;

    double gradientClip = 5.0;
};

struct CompositionNeuralTrainingResult
{
    bool trained = false;
    std::size_t epochsCompleted = 0;
    double initialLoss = 0.0;
    double finalLoss = 0.0;

    bool isValid() const noexcept;
};

CompositionNeuralTrainingResult trainCompositionNeuralModel(
    CompositionNeuralModel& model,
    const CompositionDatasetPreparedView& prepared,
    const CompositionNeuralTrainingConfig& config) noexcept;

} // namespace midigengx::music
