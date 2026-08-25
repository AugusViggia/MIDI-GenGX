#include "PhraseFingerprint.h"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace midigengx::music
{
namespace
{

int normalizePitchClass(int midi) noexcept
{
    const int pitchClass =
        midi % 12;

    return pitchClass < 0
        ? pitchClass + 12
        : pitchClass;
}

bool containsPitchClass(
    const std::vector<int>& values,
    int pitchClass) noexcept
{
    return std::find(
               values.begin(),
               values.end(),
               pitchClass) != values.end();
}

MelodicProfile classifyMelodicProfile(
    const std::vector<int>& pitches) noexcept
{
    if (pitches.size() < 2)
        return MelodicProfile::Stationary;

    int upward = 0;
    int downward = 0;
    int stationary = 0;

    for (std::size_t i = 1;
         i < pitches.size();
         ++i)
    {
        if (pitches[i] > pitches[i - 1])
            ++upward;
        else if (pitches[i] < pitches[i - 1])
            ++downward;
        else
            ++stationary;
    }

    const int motionCount =
        upward +
        downward +
        stationary;

    if (motionCount == 0)
        return MelodicProfile::Stationary;

    if (stationary == motionCount)
        return MelodicProfile::Stationary;

    if (upward > downward * 2 &&
        upward >= stationary)
    {
        return MelodicProfile::Ascending;
    }

    if (downward > upward * 2 &&
        downward >= stationary)
    {
        return MelodicProfile::Descending;
    }

    const std::size_t middle =
        pitches.size() / 2;

    bool firstHalfRises = true;
    bool secondHalfFalls = true;
    bool firstHalfFalls = true;
    bool secondHalfRises = true;

    for (std::size_t i = 1;
         i <= middle &&
         i < pitches.size();
         ++i)
    {
        if (pitches[i] < pitches[i - 1])
            firstHalfRises = false;

        if (pitches[i] > pitches[i - 1])
            firstHalfFalls = false;
    }

    for (std::size_t i = middle + 1;
         i < pitches.size();
         ++i)
    {
        if (pitches[i] > pitches[i - 1])
            secondHalfFalls = false;

        if (pitches[i] < pitches[i - 1])
            secondHalfRises = false;
    }

    if (firstHalfRises &&
        secondHalfFalls)
    {
        return MelodicProfile::Arch;
    }

    if (firstHalfFalls &&
        secondHalfRises)
    {
        return MelodicProfile::Valley;
    }

    return MelodicProfile::Mixed;
}

} // namespace

PhraseFingerprint fingerprintPhrase(
    const Phrase& phrase,
    const std::vector<int>& scalePitchClasses) noexcept
{
    PhraseFingerprint result;
    result.lengthBeats =
        phrase.lengthBeats;
    result.noteCount =
        phrase.notes.size();

    if (phrase.notes.empty())
        return result;

    std::vector<NoteEvent> sorted =
        phrase.notes;

    std::sort(
        sorted.begin(),
        sorted.end(),
        [](const NoteEvent& a, const NoteEvent& b)
        {
            if (a.startBeat == b.startBeat)
                return a.midiNote < b.midiNote;

            return a.startBeat < b.startBeat;
        });

    result.lowestMidi =
        sorted.front().midiNote;

    result.highestMidi =
        sorted.front().midiNote;

    double totalMidi = 0.0;
    double totalDuration = 0.0;
    double totalVelocity = 0.0;

    std::vector<int> pitchSequence;
    pitchSequence.reserve(
        sorted.size());

    int stepCount = 0;
    int leapCount = 0;
    int repeatedCount = 0;
    int upwardCount = 0;
    int downwardCount = 0;
    int stationaryCount = 0;

    bool monophonic = true;

    for (std::size_t i = 0;
         i < sorted.size();
         ++i)
    {
        const auto& note =
            sorted[i];

        result.lowestMidi =
            std::min(
                result.lowestMidi,
                note.midiNote);

        result.highestMidi =
            std::max(
                result.highestMidi,
                note.midiNote);

        totalMidi +=
            static_cast<double>(
                note.midiNote);

        totalDuration +=
            note.durationBeats;

        totalVelocity +=
            static_cast<double>(
                note.velocity);

        pitchSequence.push_back(
            note.midiNote);

        if (!scalePitchClasses.empty() &&
            !containsPitchClass(
                scalePitchClasses,
                normalizePitchClass(
                    note.midiNote)))
        {
            result.isScaleContained = false;
        }

        if (i > 0)
        {
            const auto& previous =
                sorted[i - 1];

            if (note.startBeat <
                previous.startBeat +
                    previous.durationBeats -
                    1.0e-9)
            {
                monophonic = false;
            }

            const int interval =
                note.midiNote -
                previous.midiNote;

            const int absoluteInterval =
                std::abs(interval);

            if (absoluteInterval == 0)
            {
                ++stationaryCount;
                ++repeatedCount;
            }
            else if (absoluteInterval <= 2)
            {
                ++stepCount;

                if (interval > 0)
                    ++upwardCount;
                else
                    ++downwardCount;
            }
            else
            {
                ++leapCount;

                if (interval > 0)
                    ++upwardCount;
                else
                    ++downwardCount;
            }
        }
    }

    result.isMonophonic =
        monophonic;

    const auto transitionCount =
        sorted.size() > 1
            ? static_cast<double>(
                  sorted.size() - 1)
            : 0.0;

    result.stepRatio =
        transitionCount > 0.0
            ? static_cast<double>(
                  stepCount) /
                  transitionCount
            : 0.0;

    result.leapRatio =
        transitionCount > 0.0
            ? static_cast<double>(
                  leapCount) /
                  transitionCount
            : 0.0;

    result.repeatedPitchRatio =
        transitionCount > 0.0
            ? static_cast<double>(
                  repeatedCount) /
                  transitionCount
            : 0.0;

    result.upwardMotionRatio =
        transitionCount > 0.0
            ? static_cast<double>(
                  upwardCount) /
                  transitionCount
            : 0.0;

    result.downwardMotionRatio =
        transitionCount > 0.0
            ? static_cast<double>(
                  downwardCount) /
                  transitionCount
            : 0.0;

    result.stationaryMotionRatio =
        transitionCount > 0.0
            ? static_cast<double>(
                  stationaryCount) /
                  transitionCount
            : 0.0;

    result.averageMidi =
        totalMidi /
        static_cast<double>(
            sorted.size());

    result.averageDurationBeats =
        totalDuration /
        static_cast<double>(
            sorted.size());

    result.averageVelocity =
        totalVelocity /
        static_cast<double>(
            sorted.size());

    if (result.lengthBeats > 0.0)
    {
        result.noteDensityPerBeat =
            static_cast<double>(
                result.noteCount) /
            result.lengthBeats;
    }

    if (result.highestMidi > result.lowestMidi)
    {
        result.registerPosition =
            (result.averageMidi -
             static_cast<double>(
                 result.lowestMidi)) /
            static_cast<double>(
                result.highestMidi -
                result.lowestMidi);
    }

    result.melodicProfile =
        classifyMelodicProfile(
            pitchSequence);

    if (scalePitchClasses.empty())
    {
        result.isScaleContained = false;
    }
    else
    {
        result.isScaleContained = true;

        for (const auto& note :
             sorted)
        {
            if (!containsPitchClass(
                    scalePitchClasses,
                    normalizePitchClass(
                        note.midiNote)))
            {
                result.isScaleContained =
                    false;
                break;
            }
        }
    }

    return result;
}

} // namespace midigengx::music
