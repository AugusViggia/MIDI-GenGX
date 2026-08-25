#pragma once

#include "../Domain/MusicalContext.h"
#include "HarmonyGuidance.h"
#include "MelodicMotion.h"

namespace midigengx::music
{

struct MelodicResolution
{
    static bool isLeap(
        int previousMidi,
        int currentMidi) noexcept;

    static int preferredResolutionDirection(
        int previousMidi,
        int currentMidi) noexcept;

    static int scoreResolution(
        int previousMidi,
        int currentMidi,
        int candidateMidi,
        int complexity,
        int tension) noexcept;

    static bool isTendencyTone(
        int pitchClass,
        int tonicPitchClass,
        const HarmonyGuidance& harmony) noexcept;
};

} // namespace midigengx::music
