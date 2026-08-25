#pragma once

#include "../Domain/MusicalContext.h"
#include "PhraseStructure.h"

#include <vector>

namespace midigengx::music
{

enum class ChordQuality
{
    Major,
    Minor,
    Diminished,
    Augmented,
    Suspended,
    Unknown
};

struct HarmonyEvent
{
    double startBeat = 0.0;
    double endBeat = 0.0;

    int scaleDegree = 0;
    int rootPitchClass = 0;

    ChordQuality quality = ChordQuality::Unknown;
    int tension = 50;

    bool isValid() const noexcept;
};

struct HarmonyPlan
{
    double totalLengthBeats = 0.0;
    std::vector<HarmonyEvent> events;

    bool isValid() const noexcept;
};

HarmonyPlan planHarmony(
    const midigengx::domain::MusicalContext& context,
    const PhraseStructurePlan& structure) noexcept;

ChordQuality inferTriadQuality(
    const std::vector<int>& scalePitchClasses,
    int scaleDegree) noexcept;

} // namespace midigengx::music
