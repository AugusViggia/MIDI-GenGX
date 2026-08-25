#pragma once

#include "CompositionModel.h"

#include <cstddef>
#include <vector>

namespace midigengx::music
{

struct CompositionNeuralPrediction
{
    std::vector<double> sectionFeatures;
    bool valid = false;

    bool isValid(
        std::size_t expectedWidth) const noexcept;
};

struct CompositionNeuralModel
{
    static constexpr int version = 1;
    static constexpr std::size_t hiddenWidth = 32;

    CompositionLearningContract contract;

    std::vector<double> inputWeights;
    std::vector<double> hiddenBias;
    std::vector<double> outputWeights;
    std::vector<double> outputBias;

    // Learned direct context-to-output residual path.
    // Zero initialization preserves the Phase 48 baseline behavior until
    // training learns a useful residual contribution.
    std::vector<double> contextResidualWeights;

    bool initialized = false;

    bool isValid() const noexcept;

    CompositionNeuralPrediction predictNextSection(
        const std::vector<double>& globalFeatures,
        const std::vector<double>& contextSectionFeatures,
        bool contextIsValid) const noexcept;
};

CompositionNeuralModel initializeCompositionNeuralModel(
    const CompositionLearningContract& contract) noexcept;

} // namespace midigengx::music
