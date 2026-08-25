#pragma once

#include "Phrase.h"

#include <cstddef>
#include <vector>

namespace midigengx::music
{

enum class MelodicProfile
{
    Stationary,
    Ascending,
    Descending,
    Arch,
    Valley,
    Mixed
};

struct PhraseFingerprint
{
    double lengthBeats = 0.0;
    std::size_t noteCount = 0;

    int lowestMidi = 0;
    int highestMidi = 0;
    double averageMidi = 0.0;
    double registerPosition = 0.0;

    double noteDensityPerBeat = 0.0;
    double averageDurationBeats = 0.0;
    double averageVelocity = 0.0;

    double stepRatio = 0.0;
    double leapRatio = 0.0;
    double repeatedPitchRatio = 0.0;

    double upwardMotionRatio = 0.0;
    double downwardMotionRatio = 0.0;
    double stationaryMotionRatio = 0.0;

    MelodicProfile melodicProfile =
        MelodicProfile::Stationary;

    bool isMonophonic = true;
    bool isScaleContained = false;
};

PhraseFingerprint fingerprintPhrase(
    const Phrase& phrase,
    const std::vector<int>& scalePitchClasses = {}) noexcept;

} // namespace midigengx::music
