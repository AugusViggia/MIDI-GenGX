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
