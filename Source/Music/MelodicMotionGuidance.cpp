#include "MelodicMotionGuidance.h"
#include "MelodicResolution.h"
#include "MelodicTendency.h"

#include <algorithm>
#include <limits>

namespace midigengx::music
{
namespace
{
int normalizePitchClass(int value) noexcept
{
    const int result = value % 12;
    return result < 0 ? result + 12 : result;
}
}

int chooseMelodicMotionPitch(
    int targetMidi,
    int previousMidi,
    int lowMidi,
    int highMidi,
    const std::vector<int>& scalePitchClasses,
    const HarmonyGuidance& harmony,
    int complexity,
    int tension,
    double beatInSection) noexcept
{
    if (scalePitchClasses.empty())
        return std::clamp(targetMidi, lowMidi, highMidi);

    int bestPitch = std::clamp(targetMidi, lowMidi, highMidi);
    int bestScore = std::numeric_limits<int>::min();
    int bestDistance = std::numeric_limits<int>::max();

    for (int midi = lowMidi; midi <= highMidi; ++midi)
    {
        const int pitchClass = normalizePitchClass(midi);

        if (std::find(
                scalePitchClasses.begin(),
                scalePitchClasses.end(),
                pitchClass) == scalePitchClasses.end())
            continue;

        const auto motion =
            MelodicMotion::analyzeInterval(previousMidi, midi);

        const int motionScore =
            MelodicMotion::scoreInterval(
                motion.semitoneDistance,
                complexity,
                tension);

        const int harmonyScore =
            harmony.scorePitch(
                pitchClass,
                beatInSection);

        const int targetDistance =
            std::abs(midi - targetMidi);

        const int resolutionScore =
            MelodicResolution::scoreResolution(
                previousMidi,
                targetMidi,
                midi,
                complexity,
                tension);

        const int targetPenalty =
            std::min(48, targetDistance * 6);

        const int score =
            harmonyScore * 2 +
            motionScore * 2 +
            resolutionScore -
            targetPenalty;

        if (score > bestScore ||
            (score == bestScore && targetDistance < bestDistance))
        {
            bestScore = score;
            bestDistance = targetDistance;
            bestPitch = midi;
        }
    }

    return bestPitch;
}


int chooseMelodicMotionPitchWithTendency(
    int targetMidi,
    int previousMidi,
    int lowMidi,
    int highMidi,
    const std::vector<int>& scalePitchClasses,
    int tonicPitchClass,
    const HarmonyGuidance& harmony,
    int complexity,
    int tension,
    int cadenceStrength,
    bool cadenceContext,
    double beatInSection) noexcept
{
    if (scalePitchClasses.empty())
        return std::clamp(
            targetMidi,
            lowMidi,
            highMidi);

    int bestPitch =
        std::clamp(
            targetMidi,
            lowMidi,
            highMidi);

    int bestScore =
        std::numeric_limits<int>::min();

    int bestDistance =
        std::numeric_limits<int>::max();

    for (int midi = lowMidi;
         midi <= highMidi;
         ++midi)
    {
        const int pitchClass =
            normalizePitchClass(midi);

        if (std::find(
                scalePitchClasses.begin(),
                scalePitchClasses.end(),
                pitchClass) ==
            scalePitchClasses.end())
        {
            continue;
        }

        const auto motion =
            MelodicMotion::analyzeInterval(
                previousMidi,
                midi);

        const int motionScore =
            MelodicMotion::scoreInterval(
                motion.semitoneDistance,
                complexity,
                tension);

        const int harmonyScore =
            harmony.scorePitch(
                pitchClass,
                beatInSection);

        const int resolutionScore =
            MelodicResolution::scoreResolution(
                previousMidi,
                targetMidi,
                midi,
                complexity,
                tension);

        const auto tendency =
            analyzeTendencyTone(
                previousMidi % 12,
                tonicPitchClass,
                scalePitchClasses,
                harmony,
                tension,
                cadenceStrength);

        const int tendencyScore =
            scoreTendencyResolution(
                pitchClass,
                tendency,
                cadenceContext);

        const int targetDistance =
            std::abs(
                midi -
                targetMidi);

        const int targetPenalty =
            std::min(
                48,
                targetDistance * 6);

        const int score =
            harmonyScore * 2 +
            motionScore * 2 +
            resolutionScore +
            tendencyScore -
            targetPenalty;

        if (score > bestScore ||
            (score == bestScore &&
             targetDistance < bestDistance))
        {
            bestScore = score;
            bestDistance = targetDistance;
            bestPitch = midi;
        }
    }

    return bestPitch;
}

} // namespace midigengx::music
