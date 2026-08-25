#pragma once

#include "HarmonyGuidance.h"
#include "MelodicMotion.h"

#include <vector>

namespace midigengx::music
{
int chooseMelodicMotionPitch(
    int targetMidi,
    int previousMidi,
    int lowMidi,
    int highMidi,
    const std::vector<int>& scalePitchClasses,
    const HarmonyGuidance& harmony,
    int complexity,
    int tension,
    double beatInSection) noexcept;

int chooseMelodicMotionPitchWithTendency(
    int targetMidi,
    int previousMidi,
    int lowMidi,
    int highMidi,
    const std::vector<int>& scalePitchClasses,
    int tonicPitchClass,
    const HarmonyGuidance& harmony,
    int complexity,
    int tension,
    int cadenceStrength,
    bool cadenceContext,
    double beatInSection) noexcept;
} // namespace midigengx::music
