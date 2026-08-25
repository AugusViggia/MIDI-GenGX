#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

namespace midigengx::music
{

struct NoteEvent
{
    int midiNote = 60;
    int velocity = 100;

    double startBeat = 0.0;
    double durationBeats = 1.0;

    int channel = 1;

    void clamp()
    {
        midiNote = std::clamp(midiNote, 0, 127);
        velocity = std::clamp(velocity, 1, 127);
        startBeat = std::max(0.0, startBeat);
        durationBeats = std::max(0.001, durationBeats);
        channel = std::clamp(channel, 1, 16);
    }
};

using NoteSequence = std::vector<NoteEvent>;

} // namespace midigengx::music
