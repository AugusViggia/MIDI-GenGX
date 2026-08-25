#pragma once

#include <cstddef>

namespace midigengx::music
{

enum class MotionDirection
{
    Stationary,
    Up,
    Down
};

enum class IntervalClass
{
    Unison,
    Step,
    SmallLeap,
    LargeLeap,
    Octave,
    Compound
};

struct MelodicMotionSample
{
    int previousMidi = 0;
    int currentMidi = 0;
    int semitoneDistance = 0;

    MotionDirection direction =
        MotionDirection::Stationary;

    IntervalClass interval =
        IntervalClass::Unison;
};

struct MelodicMotion
{
    static MelodicMotionSample analyzeInterval(
        int previousMidi,
        int currentMidi) noexcept;

    static int preferredMaximumLeap(
        int complexity,
        int tension) noexcept;

    static int scoreInterval(
        int semitoneDistance,
        int complexity,
        int tension) noexcept;
};

} // namespace midigengx::music
