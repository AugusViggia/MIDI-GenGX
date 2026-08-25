#include "PhraseAnalysis.h"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace midigengx::music
{
namespace
{

int normalizePitchClass(int midi) noexcept
{
    const int pitchClass = midi % 12;
    return pitchClass < 0
        ? pitchClass + 12
        : pitchClass;
}

} // namespace

PhraseAnalysis analyzePhrase(
    const Phrase& phrase) noexcept
{
    PhraseAnalysis analysis;
    analysis.lengthBeats = phrase.lengthBeats;
    analysis.noteCount = phrase.notes.size();

    if (phrase.notes.empty())
        return analysis;

    analysis.lowestMidi =
        phrase.notes.front().midiNote;
    analysis.highestMidi =
        phrase.notes.front().midiNote;

    double totalDuration = 0.0;
    double totalVelocity = 0.0;

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

    analysis.isMonophonic = true;

    for (std::size_t i = 0; i < sorted.size(); ++i)
    {
        const auto& note = sorted[i];

        analysis.lowestMidi =
            std::min(
                analysis.lowestMidi,
                note.midiNote);

        analysis.highestMidi =
            std::max(
                analysis.highestMidi,
                note.midiNote);

        totalDuration += note.durationBeats;
        totalVelocity +=
            static_cast<double>(note.velocity);

        analysis.pitchClasses.push_back(
            normalizePitchClass(note.midiNote));

        if (i > 0)
        {
            const auto& previous =
                sorted[i - 1];

            if (note.startBeat <
                previous.startBeat +
                    previous.durationBeats -
                    1.0e-9)
            {
                analysis.isMonophonic = false;
            }

            analysis.intervalSemitones.push_back(
                note.midiNote -
                previous.midiNote);
        }
    }

    analysis.averageDurationBeats =
        totalDuration /
        static_cast<double>(sorted.size());

    analysis.averageVelocity =
        totalVelocity /
        static_cast<double>(sorted.size());

    if (analysis.lengthBeats > 0.0)
    {
        analysis.noteDensityPerBeat =
            static_cast<double>(analysis.noteCount) /
            analysis.lengthBeats;
    }

    std::sort(
        analysis.pitchClasses.begin(),
        analysis.pitchClasses.end());

    analysis.pitchClasses.erase(
        std::unique(
            analysis.pitchClasses.begin(),
            analysis.pitchClasses.end()),
        analysis.pitchClasses.end());

    return analysis;
}

bool isPhraseContainedInRange(
    const Phrase& phrase,
    int lowMidi,
    int highMidi) noexcept
{
    lowMidi = std::clamp(lowMidi, 0, 127);
    highMidi = std::clamp(highMidi, 0, 127);

    if (lowMidi > highMidi)
        std::swap(lowMidi, highMidi);

    for (const auto& note : phrase.notes)
    {
        if (note.midiNote < lowMidi ||
            note.midiNote > highMidi)
        {
            return false;
        }
    }

    return true;
}

bool isPhraseScaleContained(
    const Phrase& phrase,
    const std::vector<int>& pitchClasses) noexcept
{
    if (pitchClasses.empty())
        return phrase.notes.empty();

    for (const auto& note : phrase.notes)
    {
        const int pitchClass =
            normalizePitchClass(note.midiNote);

        if (std::find(
                pitchClasses.begin(),
                pitchClasses.end(),
                pitchClass) == pitchClasses.end())
        {
            return false;
        }
    }

    return true;
}

} // namespace midigengx::music
