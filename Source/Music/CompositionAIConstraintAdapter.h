#pragma once

#include "CompositionInferencePipeline.h"

#include <cstddef>
#include <vector>

namespace midigengx::music
{

struct CompositionAIConstraintRequest
{
    std::vector<double> globalFeatures;
    std::vector<double> contextSectionFeatures;
    bool contextIsValid = false;

    bool isValid(
        const CompositionLearningContract& contract) const noexcept;
};

struct CompositionAIConstraintProfile
{
    double roleTarget = 0.0;
    double tensionTarget = 0.0;
    double tensionDeltaTarget = 0.0;
    double harmonyDegreeTarget = 0.0;
    double harmonyQualityTarget = 0.0;
    double harmonicDegreeDeltaTarget = 0.0;

    double confidence = 0.0;
    bool valid = false;

    bool isValid() const noexcept;
};

struct CompositionAIConstraintResult
{
    CompositionAIConstraintProfile profile;

    bool valid = false;

    bool isValid() const noexcept;
};

CompositionAIConstraintResult adaptAIResultToMusicalConstraints(
    const CompositionInferencePipeline& pipeline,
    const CompositionAIConstraintRequest& request) noexcept;

} // namespace midigengx::music
