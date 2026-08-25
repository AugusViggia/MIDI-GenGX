#pragma once

#include "HarmonyPlan.h"
#include "NoteEvent.h"

#include <vector>

namespace midigengx::music
{

struct HarmonyGuidance
{
    std::vector<int> chordPitchClasses;
    int tension = 50;
    int strength = 50;

    bool isChordTone(int pitchClass) const noexcept;

    int scorePitch(
        int pitchClass,
        double beatInSection) const noexcept;
};

const HarmonyEvent* findHarmonyEventAtBeat(
    const HarmonyPlan& plan,
    double beat) noexcept;

HarmonyGuidance buildHarmonyGuidance(
    const HarmonyPlan& plan,
    const HarmonyEvent& event,
    const std::vector<int>& scalePitchClasses,
    int strength) noexcept;

int chooseHarmonyAwarePitch(
    int targetMidi,
    int lowMidi,
    int highMidi,
    const std::vector<int>& scalePitchClasses,
    const HarmonyGuidance& guidance,
    double beatInSection) noexcept;

} // namespace midigengx::music
