#include "CompositionAIEngineBridge.h"

#include <cmath>

namespace midigengx::music
{

bool CompositionAIGuidance::isValid() const noexcept
{
    const auto values = {
        roleTarget,
        tensionTarget,
        tensionDeltaTarget,
        harmonyDegreeTarget,
        harmonyQualityTarget,
        harmonicDegreeDeltaTarget,
        densityTarget,
        catchinessTarget,
        syncopationTarget,
        octaveMovementTarget,
        variationTarget,
        repetitionTarget,
        tensionSelectorTarget,
        complexityTarget,
        humanizationTarget,
        noteLengthVariationTarget,
        cadenceStrengthTarget
    };

    for (const auto value :
         values)
    {
        if (!std::isfinite(value) ||
            value < -1.0 ||
            value > 1.0)
        {
            return false;
        }
    }

    return valid &&
           std::isfinite(confidence) &&
           confidence >= 0.0 &&
           confidence <= 1.0;
}

bool CompositionAIEngineBridge::isValid() const noexcept
{
    return enabled;
}

CompositionAIGuidance
CompositionAIEngineBridge::deriveGuidance(
    const CompositionAIConstraintProfile& profile) const noexcept
{
    CompositionAIGuidance guidance;

    if (!isValid() ||
        !profile.isValid())
    {
        return guidance;
    }

    guidance.roleTarget =
        profile.roleTarget;

    guidance.tensionTarget =
        profile.tensionTarget;

    guidance.tensionDeltaTarget =
        profile.tensionDeltaTarget;

    guidance.harmonyDegreeTarget =
        profile.harmonyDegreeTarget;

    guidance.harmonyQualityTarget =
        profile.harmonyQualityTarget;

    guidance.harmonicDegreeDeltaTarget =
        profile.harmonicDegreeDeltaTarget;

    guidance.confidence =
        profile.confidence;

    guidance.valid =
        true;

    return guidance;
}

CompositionAIEngineBridge
buildCompositionAIEngineBridge(
    bool enabled) noexcept
{
    CompositionAIEngineBridge bridge;

    bridge.enabled =
        enabled;

    return bridge;
}

} // namespace midigengx::music
