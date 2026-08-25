#pragma once

#include "CompositionMidiSectionAnalyzer.h"
#include "HarmonyPlan.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace midigengx::music
{

enum class CompositionMidiScale
{
    Unknown,
    Major,
    Minor
};

struct CompositionMidiKeyEstimate
{
    int tonicPitchClass = 0;
    CompositionMidiScale scale =
        CompositionMidiScale::Unknown;

    double confidence = 0.0;
    bool valid = false;

    bool isValid() const noexcept;
};

struct CompositionMidiSectionHarmony
{
    std::size_t sectionIndex = 0;

    int rootPitchClass = 0;
    int scaleDegree = 0;

    ChordQuality quality =
        ChordQuality::Unknown;

    double confidence = 0.0;

    bool valid = false;

    bool isValid() const noexcept;
};

struct CompositionMidiHarmonyAnalysis
{
    static constexpr int version = 1;

    CompositionMidiKeyEstimate key;
    std::vector<CompositionMidiSectionHarmony> sections;

    bool valid = false;

    bool isValid() const noexcept;
};

CompositionMidiHarmonyAnalysis analyzeCompositionMidiHarmony(
    const CompositionMidiCorpusRecord& record,
    const CompositionMidiSectionAnalysis& sections) noexcept;

} // namespace midigengx::music
