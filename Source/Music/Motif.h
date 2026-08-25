#pragma once

#include "Phrase.h"

#include <cstddef>
#include <vector>

namespace midigengx::music
{

struct MotifNote
{
    double startBeat = 0.0;
    double durationBeats = 0.0;

    // Interval relative to the motif's first note. This makes the motif
    // transposition-independent and suitable for later style/knowledge
    // analysis.
    int relativePitch = 0;

    int velocityDelta = 0;
};

struct Motif
{
    std::vector<MotifNote> notes;
    double lengthBeats = 0.0;

    bool isValid() const noexcept;

    Motif transposedToFirstPitch(int midiPitch) const noexcept;
};

Motif extractMotif(
    const Phrase& phrase,
    double maxLengthBeats) noexcept;

Phrase applyMotif(
    const Motif& motif,
    int firstMidiPitch,
    double startBeat,
    int channel = 1) noexcept;

} // namespace midigengx::music
