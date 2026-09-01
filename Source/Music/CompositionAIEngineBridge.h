#pragma once

#include "CompositionAIConstraintAdapter.h"

#include <cstddef>
#include <vector>

namespace midigengx::music
{

struct CompositionAIGuidance
{
    double roleTarget = 0.0;
    double tensionTarget = 0.0;
    double tensionDeltaTarget = 0.0;
    double harmonyDegreeTarget = 0.0;
    double harmonyQualityTarget = 0.0;
    double harmonicDegreeDeltaTarget = 0.0;

    // Explicit user selector intent. Normalized to [-1, 1] for 0-100% selector values.
    double densityTarget = 0.0;
    double catchinessTarget = 0.0;
    double syncopationTarget = 0.0;
    double octaveMovementTarget = 0.0;
    double variationTarget = 0.0;
    double repetitionTarget = 0.0;
    double tensionSelectorTarget = 0.0;
    double complexityTarget = 0.0;
    double humanizationTarget = -1.0;
    double noteLengthVariationTarget = -1.0;
    double cadenceStrengthTarget = 0.5;

    double confidence = 0.0;
    bool valid = false;

    bool isValid() const noexcept;
};

struct CompositionAIEngineBridge
{
    static constexpr int version = 1;

    bool enabled = false;

    bool isValid() const noexcept;

    CompositionAIGuidance deriveGuidance(
        const CompositionAIConstraintProfile& profile) const noexcept;
};

CompositionAIEngineBridge buildCompositionAIEngineBridge(
    bool enabled = true) noexcept;

} // namespace midigengx::music
