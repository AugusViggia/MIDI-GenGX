#pragma once

#include "CompositionModelEvaluation.h"

#include <cstddef>
#include <vector>

namespace midigengx::music
{

struct CompositionMusicalEvaluationResult
{
    std::size_t exampleCount = 0;

    double structuralCoherenceScore = 0.0;
    double tensionConsistencyScore = 0.0;
    double harmonicConsistencyScore = 0.0;
    double rangeValidityScore = 0.0;
    double overallScore = 0.0;

    bool valid = false;

    bool isValid() const noexcept;
};

CompositionMusicalEvaluationResult evaluateCompositionNeuralMusicalQuality(
    const CompositionNeuralModel& model,
    const CompositionDatasetPreparedView& prepared,
    bool useValidationSet) noexcept;

} // namespace midigengx::music
