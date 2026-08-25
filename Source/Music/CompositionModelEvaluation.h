#pragma once

#include "CompositionNeuralModel.h"
#include "CompositionModel.h"

#include <cstddef>

namespace midigengx::music
{

struct CompositionModelEvaluationResult
{
    std::size_t sampleCount = 0;
    std::size_t exampleCount = 0;

    double meanSquaredError = 0.0;
    double meanAbsoluteError = 0.0;

    bool valid = false;

    bool isValid() const noexcept;
};

CompositionModelEvaluationResult evaluateCompositionBaselineModel(
    const CompositionModel& model,
    const CompositionDatasetPreparedView& prepared,
    bool useValidationSet) noexcept;

CompositionModelEvaluationResult evaluateCompositionNeuralModel(
    const CompositionNeuralModel& model,
    const CompositionDatasetPreparedView& prepared,
    bool useValidationSet) noexcept;

} // namespace midigengx::music
