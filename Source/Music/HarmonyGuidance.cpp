#include "HarmonyGuidance.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace midigengx::music
{
namespace
{

int normalizePitchClass(int value) noexcept
{
    const int pc = value % 12;
    return pc < 0 ? pc + 12 : pc;
}

std::vector<int> buildTriadPitchClasses(
    const HarmonyEvent& event,
    const std::vector<int>& scalePitchClasses)
{
    std::vector<int> result;

    if (scalePitchClasses.empty())
        return result;

    const int size =
        static_cast<int>(scalePitchClasses.size());

    const int degree =
        std::clamp(
            event.scaleDegree,
            0,
            size - 1);

    result.push_back(
        scalePitchClasses[
            static_cast<std::size_t>(
                degree)]);

    if (size >= 3)
    {
        result.push_back(
            scalePitchClasses[
                static_cast<std::size_t>(
                    (degree + 2) % size)]);

        result.push_back(
            scalePitchClasses[
                static_cast<std::size_t>(
                    (degree + 4) % size)]);
    }

    if (event.quality == ChordQuality::Suspended &&
        size >= 4)
    {
        result.erase(
            result.begin() + 1);

        result.push_back(
            scalePitchClasses[
                static_cast<std::size_t>(
                    (degree + 3) % size)]);
    }

    return result;
}

} // namespace

bool HarmonyGuidance::isChordTone(
    int pitchClass) const noexcept
{
    const int normalized =
        normalizePitchClass(pitchClass);

    return std::find(
        chordPitchClasses.begin(),
        chordPitchClasses.end(),
        normalized) != chordPitchClasses.end();
}

int HarmonyGuidance::scorePitch(
    int pitchClass,
    double beatInSection) const noexcept
{
    const bool chordTone =
        isChordTone(pitchClass);

    const double beatPhase =
        std::fmod(
            std::max(0.0, beatInSection),
            4.0);

    const bool strongBeat =
        beatPhase < 1.0e-6 ||
        std::abs(beatPhase - 2.0) < 1.0e-6;

    // Harmonic strength is highest on strong beats and decreases as the
    // requested tension rises. This creates a controlled consonance/tension
    // relationship without random decisions.
    const int consonantScore =
        chordTone
            ? (strongBeat ? 100 : 70)
            : (strongBeat ? 35 : 55);

    const int tensionAdjustment =
        static_cast<int>(
            std::lround(
                static_cast<double>(tension) *
                (chordTone ? -0.20 : 0.20)));

    const int strengthAdjustment =
        static_cast<int>(
            std::lround(
                static_cast<double>(strength) *
                (chordTone ? 0.25 : -0.15)));

    return std::clamp(
        consonantScore +
            tensionAdjustment +
            strengthAdjustment,
        0,
        100);
}

const HarmonyEvent* findHarmonyEventAtBeat(
    const HarmonyPlan& plan,
    double beat) noexcept
{
    for (const auto& event : plan.events)
    {
        if (beat + 1.0e-9 >= event.startBeat &&
            beat < event.endBeat - 1.0e-9)
        {
            return &event;
        }
    }

    if (!plan.events.empty() &&
        beat >= plan.events.back().endBeat)
    {
        return &plan.events.back();
    }

    return nullptr;
}

HarmonyGuidance buildHarmonyGuidance(
    const HarmonyPlan&,
    const HarmonyEvent& event,
    const std::vector<int>& scalePitchClasses,
    int strength) noexcept
{
    HarmonyGuidance guidance;
    guidance.chordPitchClasses =
        buildTriadPitchClasses(
            event,
            scalePitchClasses);

    guidance.tension =
        std::clamp(
            event.tension,
            0,
            100);

    guidance.strength =
        std::clamp(
            strength,
            0,
            100);

    return guidance;
}

int chooseHarmonyAwarePitch(
    int targetMidi,
    int lowMidi,
    int highMidi,
    const std::vector<int>& scalePitchClasses,
    const HarmonyGuidance& guidance,
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

        const int harmonicScore =
            guidance.scorePitch(
                pitchClass,
                beatInSection);

        const int distance =
            std::abs(
                midi - targetMidi);

        const int proximityPenalty =
            std::min(
                60,
                distance * 8);

        const int score =
            harmonicScore -
            proximityPenalty;

        if (score > bestScore ||
            (score == bestScore &&
             distance < bestDistance))
        {
            bestScore = score;
            bestDistance = distance;
            bestPitch = midi;
        }
    }

    return bestPitch;
}

} // namespace midigengx::music
