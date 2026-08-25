#include "MelodicMotion.h"

#include <algorithm>
#include <cmath>

namespace midigengx::music
{
namespace
{

IntervalClass classifyDistance(
    int distance) noexcept
{
    if (distance <= 0)
        return IntervalClass::Unison;

    if (distance <= 2)
        return IntervalClass::Step;

    if (distance <= 5)
        return IntervalClass::SmallLeap;

    if (distance == 12)
        return IntervalClass::Octave;

    if (distance <= 11)
        return IntervalClass::LargeLeap;

    return IntervalClass::Compound;
}

} // namespace

MelodicMotionSample MelodicMotion::analyzeInterval(
    int previousMidi,
    int currentMidi) noexcept
{
    MelodicMotionSample result;

    result.previousMidi = previousMidi;
    result.currentMidi = currentMidi;
    result.semitoneDistance =
        std::abs(
            currentMidi -
            previousMidi);

    if (currentMidi > previousMidi)
    {
        result.direction =
            MotionDirection::Up;
    }
    else if (currentMidi < previousMidi)
    {
        result.direction =
            MotionDirection::Down;
    }
    else
    {
        result.direction =
            MotionDirection::Stationary;
    }

    result.interval =
        classifyDistance(
            result.semitoneDistance);

    return result;
}

int MelodicMotion::preferredMaximumLeap(
    int complexity,
    int tension) noexcept
{
    complexity =
        std::clamp(
            complexity,
            0,
            100);

    tension =
        std::clamp(
            tension,
            0,
            100);

    // 2–7 semitones is the intentional core range. Complexity and tension
    // progressively widen the acceptable interval envelope.
    const int base =
        2 +
        static_cast<int>(
            std::lround(
                complexity *
                0.03));

    const int tensionAllowance =
        static_cast<int>(
            std::lround(
                tension *
                0.02));

    return std::clamp(
        base + tensionAllowance,
        2,
        7);
}

int MelodicMotion::scoreInterval(
    int semitoneDistance,
    int complexity,
    int tension) noexcept
{
    if (semitoneDistance < 0)
        return 0;

    complexity =
        std::clamp(
            complexity,
            0,
            100);

    tension =
        std::clamp(
            tension,
            0,
            100);

    const int preferred =
        preferredMaximumLeap(
            complexity,
            tension);

    if (semitoneDistance == 0)
        return 62;

    if (semitoneDistance <= 2)
        return 100;

    if (semitoneDistance <= preferred)
        return std::max(
            75,
            100 -
                (semitoneDistance -
                 2) *
                6);

    if (semitoneDistance == 12)
        return 55;

    const int excess =
        semitoneDistance -
        preferred;

    return std::clamp(
        70 -
            excess *
            12 +
            complexity / 4 +
            tension / 5,
        0,
        70);
}

} // namespace midigengx::music
