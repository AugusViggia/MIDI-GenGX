#include "MelodicResolution.h"

#include <algorithm>
#include <cmath>

namespace midigengx::music
{
namespace
{

int normalizePitchClass(int value) noexcept
{
    const int result = value % 12;
    return result < 0 ? result + 12 : result;
}

} // namespace

bool MelodicResolution::isLeap(
    int previousMidi,
    int currentMidi) noexcept
{
    const int distance =
        std::abs(
            currentMidi -
            previousMidi);

    return distance > 2;
}

int MelodicResolution::preferredResolutionDirection(
    int previousMidi,
    int currentMidi) noexcept
{
    if (!isLeap(previousMidi, currentMidi))
        return 0;

    if (currentMidi > previousMidi)
        return -1;

    if (currentMidi < previousMidi)
        return 1;

    return 0;
}

int MelodicResolution::scoreResolution(
    int previousMidi,
    int currentMidi,
    int candidateMidi,
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

    if (!isLeap(previousMidi, currentMidi))
        return 0;

    const int preferredDirection =
        preferredResolutionDirection(
            previousMidi,
            currentMidi);

    const int candidateDelta =
        candidateMidi -
        currentMidi;

    if (candidateDelta == 0)
        return -30;

    const int candidateDirection =
        candidateDelta > 0
            ? 1
            : -1;

    const int stepDistance =
        std::abs(candidateDelta);

    int score = 0;

    if (candidateDirection == preferredDirection)
        score += 60;

    if (stepDistance <= 2)
        score += 35;
    else if (stepDistance <= 4)
        score += 15;
    else
        score -= std::min(
            35,
            stepDistance * 5);

    score +=
        static_cast<int>(
            std::lround(
                complexity *
                0.10));

    score +=
        static_cast<int>(
            std::lround(
                tension *
                0.05));

    return std::clamp(
        score,
        -100,
        100);
}

bool MelodicResolution::isTendencyTone(
    int pitchClass,
    int tonicPitchClass,
    const HarmonyGuidance& harmony) noexcept
{
    const int pc =
        normalizePitchClass(
            pitchClass);

    const int tonic =
        normalizePitchClass(
            tonicPitchClass);

    const int leadingTone =
        normalizePitchClass(
            tonic - 1);

    if (pc == leadingTone)
        return true;

    if (!harmony.isChordTone(pc))
        return false;

    return false;
}

} // namespace midigengx::music
