#include "MelodicTendency.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace midigengx::music
{
namespace
{

int normalizePitchClass(int value) noexcept
{
    const int result = value % 12;
    return result < 0 ? result + 12 : result;
}

int findScaleDegree(
    int pitchClass,
    const std::vector<int>& scalePitchClasses) noexcept
{
    const int normalized =
        normalizePitchClass(pitchClass);

    for (std::size_t i = 0;
         i < scalePitchClasses.size();
         ++i)
    {
        if (normalizePitchClass(
                scalePitchClasses[i]) ==
            normalized)
        {
            return static_cast<int>(i);
        }
    }

    return -1;
}

} // namespace

TendencyToneInfo analyzeTendencyTone(
    int pitchClass,
    int tonicPitchClass,
    const std::vector<int>& scalePitchClasses,
    const HarmonyGuidance& harmony,
    int tension,
    int cadenceStrength) noexcept
{
    TendencyToneInfo info;

    info.pitchClass =
        normalizePitchClass(
            pitchClass);

    const int tonic =
        normalizePitchClass(
            tonicPitchClass);

    tension =
        std::clamp(
            tension,
            0,
            100);

    cadenceStrength =
        std::clamp(
            cadenceStrength,
            0,
            100);

    if (scalePitchClasses.empty())
        return info;

    const int tonicDegree =
        findScaleDegree(
            tonic,
            scalePitchClasses);

    if (tonicDegree < 0)
        return info;

    const int leadingTone =
        normalizePitchClass(
            tonic - 1);

    // A chromatic leading tone is a valid tendency even when the selected
    // scale is natural minor or another collection that does not contain
    // the semitone below the tonic. Tendency analysis intentionally happens
    // before the diatonic-scale membership check.
    if (info.pitchClass == leadingTone)
    {
        info.direction =
            TendencyDirection::ResolveUp;

        info.targetPitchClass =
            tonic;

        info.strength =
            std::clamp(
                55 +
                    tension / 4 +
                    cadenceStrength / 5,
                0,
                100);

        return info;
    }

    const int degree =
        findScaleDegree(
            info.pitchClass,
            scalePitchClasses);

    if (degree < 0)
        return info;

    const int size =
        static_cast<int>(
            scalePitchClasses.size());

    // Chord tones are normally stable. Non-chord scale degrees can acquire
    // directional pressure when tension/cadence are high.
    if (harmony.isChordTone(info.pitchClass))
        return info;

    const int nextDegree =
        (degree + 1) % size;

    const int previousDegree =
        (degree - 1 + size) % size;

    const int upwardTarget =
        normalizePitchClass(
            scalePitchClasses[
                static_cast<std::size_t>(
                    nextDegree)]);

    const int downwardTarget =
        normalizePitchClass(
            scalePitchClasses[
                static_cast<std::size_t>(
                    previousDegree)]);

    const int upwardDistance =
        std::abs(
            normalizePitchClass(
                upwardTarget -
                info.pitchClass));

    const int downwardDistance =
        std::abs(
            normalizePitchClass(
                info.pitchClass -
                downwardTarget));

    const int pressure =
        std::clamp(
            tension / 2 +
            cadenceStrength / 3,
            0,
            100);

    if (pressure >= 70)
    {
        if (upwardDistance <= downwardDistance)
        {
            info.direction =
                TendencyDirection::ResolveUp;
            info.targetPitchClass =
                upwardTarget;
        }
        else
        {
            info.direction =
                TendencyDirection::ResolveDown;
            info.targetPitchClass =
                downwardTarget;
        }

        info.strength =
            std::clamp(
                pressure - 20,
                0,
                80);
    }

    return info;
}

int scoreTendencyResolution(
    int candidatePitchClass,
    const TendencyToneInfo& activeTendency,
    bool cadenceContext) noexcept
{
    if (!activeTendency.isTendency())
        return 0;

    const int candidate =
        normalizePitchClass(
            candidatePitchClass);

    const int target =
        normalizePitchClass(
            activeTendency.targetPitchClass);

    if (candidate == target)
    {
        return std::clamp(
            activeTendency.strength +
                (cadenceContext ? 20 : 0),
            0,
            100);
    }

    if (candidate ==
        activeTendency.pitchClass)
    {
        return cadenceContext
            ? -activeTendency.strength / 2
            : 0;
    }

    return 0;
}

} // namespace midigengx::music
