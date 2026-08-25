#include "CompositionAIConstraintAdapter.h"

#include <algorithm>
#include <cmath>

namespace midigengx::music
{

bool CompositionAIConstraintRequest::isValid(
    const CompositionLearningContract& contract) const noexcept
{
    return contract.isValid() &&
           globalFeatures.size() ==
               contract.globalInputWidth &&
           contextSectionFeatures.size() ==
               contract.sectionInputWidth &&
           contextIsValid;
}

bool CompositionAIConstraintProfile::isValid() const noexcept
{
    const auto values = {
        roleTarget,
        tensionTarget,
        tensionDeltaTarget,
        harmonyDegreeTarget,
        harmonyQualityTarget,
        harmonicDegreeDeltaTarget
    };

    for (const auto value : values)
    {
        if (!std::isfinite(value) ||
            value < -1.0 ||
            value > 1.0)
        {
            return false;
        }
    }

    return std::isfinite(confidence) &&
           confidence >= 0.0 &&
           confidence <= 1.0 &&
           valid;
}

bool CompositionAIConstraintResult::isValid() const noexcept
{
    return valid &&
           profile.isValid();
}

CompositionAIConstraintResult
adaptAIResultToMusicalConstraints(
    const CompositionInferencePipeline& pipeline,
    const CompositionAIConstraintRequest& request) noexcept
{
    CompositionAIConstraintResult result;

    if (!pipeline.isValid() ||
        !request.isValid(
            pipeline.contract))
    {
        return result;
    }

    const auto inference =
        pipeline.infer(
            CompositionInferenceRequest{
                request.globalFeatures,
                request.contextSectionFeatures,
                request.contextIsValid
            });

    if (!inference.isValid(
            pipeline.contract))
    {
        return result;
    }

    const auto& values =
        inference.prediction.sectionFeatures;

    if (values.size() != 6)
        return result;

    result.profile.roleTarget =
        values[0];

    result.profile.tensionTarget =
        values[1];

    result.profile.tensionDeltaTarget =
        values[2];

    result.profile.harmonyDegreeTarget =
        values[3];

    result.profile.harmonyQualityTarget =
        values[4];

    result.profile.harmonicDegreeDeltaTarget =
        values[5];

    // Confidence is deliberately conservative in this integration phase.
    // It measures prediction validity, not perceptual musical quality.
    result.profile.confidence =
        1.0;

    result.profile.valid =
        true;

    result.valid =
        result.profile.isValid();

    return result;
}

} // namespace midigengx::music
