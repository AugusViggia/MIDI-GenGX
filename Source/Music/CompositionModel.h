#pragma once

#include "CompositionLearningContract.h"

#include <cstddef>
#include <vector>

namespace midigengx::music
{

struct CompositionModelPrediction
{
    std::vector<double> sectionFeatures;
    bool valid = false;

    bool isValid(
        std::size_t expectedWidth) const noexcept;
};

struct CompositionModel
{
    static constexpr int version = 1;

    CompositionLearningContract contract;

    // Phase 42 baseline parameters:
    // learned mean next-section feature vector from training examples.
    std::vector<double> nextSectionMean;

    bool trained = false;

    bool isValid() const noexcept;

    CompositionModelPrediction predictNextSection(
        const std::vector<double>& globalFeatures,
        const std::vector<double>& contextSectionFeatures,
        bool contextIsValid) const noexcept;
};

CompositionModel trainCompositionBaselineModel(
    const CompositionDatasetPreparedView& prepared,
    const CompositionLearningContract& contract) noexcept;

} // namespace midigengx::music
