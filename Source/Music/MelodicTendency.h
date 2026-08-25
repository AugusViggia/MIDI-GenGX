#pragma once

#include "HarmonyGuidance.h"

#include <vector>

namespace midigengx::music
{

enum class TendencyDirection
{
    None,
    ResolveUp,
    ResolveDown
};

struct TendencyToneInfo
{
    int pitchClass = 0;
    TendencyDirection direction = TendencyDirection::None;
    int targetPitchClass = 0;
    int strength = 0;

    bool isTendency() const noexcept
    {
        return direction != TendencyDirection::None &&
               strength > 0;
    }
};

TendencyToneInfo analyzeTendencyTone(
    int pitchClass,
    int tonicPitchClass,
    const std::vector<int>& scalePitchClasses,
    const HarmonyGuidance& harmony,
    int tension,
    int cadenceStrength) noexcept;

int scoreTendencyResolution(
    int candidatePitchClass,
    const TendencyToneInfo& activeTendency,
    bool cadenceContext) noexcept;

} // namespace midigengx::music
