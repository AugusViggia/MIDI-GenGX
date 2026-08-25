#include "Motif.h"

#include <algorithm>
#include <cmath>

namespace midigengx::music
{
namespace
{

constexpr double kEpsilon = 1.0e-9;

} // namespace

bool Motif::isValid() const noexcept
{
    if (lengthBeats <= 0.0)
        return false;

    double previousStart = -1.0;

    for (const auto& note : notes)
    {
        if (note.startBeat < 0.0 ||
            note.durationBeats <= 0.0)
        {
            return false;
        }

        if (note.startBeat + note.durationBeats >
            lengthBeats + kEpsilon)
        {
            return false;
        }

        if (previousStart >= 0.0 &&
            note.startBeat + kEpsilon <
                previousStart)
        {
            return false;
        }

        previousStart = note.startBeat;
    }

    return true;
}

Motif Motif::transposedToFirstPitch(
    int) const noexcept
{
    Motif result = *this;

    // relativePitch already describes intervals from the motif origin, so
    // changing the first pitch does not alter the motif's internal identity.

    return result;
}

Motif extractMotif(
    const Phrase& phrase,
    double maxLengthBeats) noexcept
{
    Motif motif;

    if (maxLengthBeats <= 0.0 ||
        phrase.notes.empty())
    {
        return motif;
    }

    auto sortedNotes = phrase.notes;

    std::sort(
        sortedNotes.begin(),
        sortedNotes.end(),
        [](const NoteEvent& a, const NoteEvent& b)
        {
            if (a.startBeat == b.startBeat)
                return a.midiNote < b.midiNote;

            return a.startBeat < b.startBeat;
        });

    const int originPitch =
        sortedNotes.front().midiNote;

    for (const auto& source : sortedNotes)
    {
        if (source.startBeat >= maxLengthBeats)
            break;

        const double available =
            maxLengthBeats - source.startBeat;

        if (available <= kEpsilon)
            break;

        MotifNote note;
        note.startBeat = source.startBeat;
        note.durationBeats =
            std::min(
                source.durationBeats,
                available);
        note.relativePitch =
            source.midiNote - originPitch;
        note.velocityDelta =
            source.velocity -
            sortedNotes.front().velocity;

        motif.notes.push_back(note);
    }

    motif.lengthBeats =
        std::min(
            maxLengthBeats,
            sortedNotes.empty()
                ? 0.0
                : sortedNotes.back().startBeat +
                  sortedNotes.back().durationBeats);

    if (!motif.notes.empty())
    {
        motif.lengthBeats =
            std::max(
                motif.lengthBeats,
                motif.notes.back().startBeat +
                    motif.notes.back().durationBeats);
    }

    return motif;
}

Phrase applyMotif(
    const Motif& motif,
    int firstMidiPitch,
    double startBeat,
    int channel) noexcept
{
    Phrase phrase;

    if (!motif.isValid())
        return phrase;

    const int safePitch =
        std::clamp(firstMidiPitch, 0, 127);

    const int safeChannel =
        std::clamp(channel, 1, 16);

    for (const auto& motifNote : motif.notes)
    {
        NoteEvent note;
        note.midiNote =
            std::clamp(
                safePitch + motifNote.relativePitch,
                0,
                127);

        note.velocity =
            std::clamp(
                100 + motifNote.velocityDelta,
                1,
                127);

        note.startBeat =
            startBeat +
            motifNote.startBeat;

        note.durationBeats =
            motifNote.durationBeats;

        note.channel =
            safeChannel;

        phrase.notes.push_back(note);
    }

    phrase.lengthBeats =
        startBeat +
        motif.lengthBeats;

    phrase.normalize();
    return phrase;
}

} // namespace midigengx::music
