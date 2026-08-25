#pragma once

#include "NoteEvent.h"

#include <algorithm>
#include <cstddef>

namespace midigengx::music
{

struct Phrase
{
    NoteSequence notes;
    double lengthBeats = 16.0;

    void normalize()
    {
        for (auto& note : notes)
            note.clamp();

        std::sort(
            notes.begin(),
            notes.end(),
            [](const NoteEvent& a, const NoteEvent& b)
            {
                if (a.startBeat == b.startBeat)
                    return a.midiNote < b.midiNote;

                return a.startBeat < b.startBeat;
            });

        lengthBeats =
            std::max(
                lengthBeats,
                notes.empty()
                    ? 0.0
                    : notes.back().startBeat +
                      notes.back().durationBeats);
    }

    bool isValid() const noexcept
    {
        if (lengthBeats <= 0.0)
            return false;

        for (const auto& note : notes)
        {
            if (note.midiNote < 0 || note.midiNote > 127)
                return false;

            if (note.velocity < 1 || note.velocity > 127)
                return false;

            if (note.startBeat < 0.0)
                return false;

            if (note.durationBeats <= 0.0)
                return false;

            if (note.channel < 1 || note.channel > 16)
                return false;

            if (note.startBeat + note.durationBeats > lengthBeats + 1.0e-9)
                return false;
        }

        return true;
    }
};

} // namespace midigengx::music
