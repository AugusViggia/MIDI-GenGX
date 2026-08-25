#pragma once

#include "Phrase.h"

#include <cstddef>
#include <vector>

namespace midigengx::music
{

struct PhraseAnalysis
{
    double lengthBeats = 0.0;
    std::size_t noteCount = 0;

    int lowestMidi = 0;
    int highestMidi = 0;

    double noteDensityPerBeat = 0.0;
    double averageDurationBeats = 0.0;
    double averageVelocity = 0.0;

    std::vector<int> pitchClasses;
    std::vector<int> intervalSemitones;

    bool isMonophonic = true;
    bool isScaleContained = false;
};

PhraseAnalysis analyzePhrase(
    const Phrase& phrase) noexcept;

bool isPhraseContainedInRange(
    const Phrase& phrase,
    int lowMidi,
    int highMidi) noexcept;

bool isPhraseScaleContained(
    const Phrase& phrase,
    const std::vector<int>& pitchClasses) noexcept;

} // namespace midigengx::music
