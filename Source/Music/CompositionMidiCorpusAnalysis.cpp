#include "CompositionMidiCorpusAnalysis.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace midigengx::music
{
namespace
{

bool finite(
    double value) noexcept
{
    return std::isfinite(value);
}

double durationBeats(
    const CompositionMidiCorpusRecord& record,
    const CompositionMidiNote& note) noexcept
{
    return static_cast<double>(
               note.endTick -
               note.startTick) /
           static_cast<double>(
               record.ticksPerQuarterNote);
}

} // namespace

bool CompositionMidiCorpusAnalysis::isValid()
    const noexcept
{
    return valid &&
           finite(notesPerBeat) &&
           finite(averagePitch) &&
           finite(pitchRange) &&
           finite(averageVelocity) &&
           finite(averageDurationBeats) &&
           finite(maxPolyphony) &&
           notesPerBeat >= 0.0 &&
           averagePitch >= 0.0 &&
           averagePitch <= 127.0 &&
           pitchRange >= 0.0 &&
           pitchRange <= 127.0 &&
           averageVelocity >= 0.0 &&
           averageVelocity <= 127.0 &&
           averageDurationBeats > 0.0 &&
           maxPolyphony >= 1.0;
}

CompositionMidiCorpusAnalysis
analyzeCompositionMidiCorpus(
    const CompositionMidiCorpusRecord& record)
    noexcept
{
    CompositionMidiCorpusAnalysis analysis;

    if (!record.isValid())
        return analysis;

    const auto noteCount =
        record.notes.size();

    if (noteCount == 0 ||
        record.ticksPerQuarterNote == 0 ||
        record.lengthTicks == 0)
    {
        return analysis;
    }

    std::uint8_t minimumPitch = 127;
    std::uint8_t maximumPitch = 0;

    double pitchSum = 0.0;
    double velocitySum = 0.0;
    double durationSum = 0.0;

    struct Boundary
    {
        std::uint32_t tick = 0;
        bool start = false;
    };

    std::vector<Boundary> boundaries;
    boundaries.reserve(
        noteCount * 2);

    for (const auto& note :
         record.notes)
    {
        minimumPitch =
            std::min(
                minimumPitch,
                note.midiNote);

        maximumPitch =
            std::max(
                maximumPitch,
                note.midiNote);

        pitchSum +=
            static_cast<double>(
                note.midiNote);

        velocitySum +=
            static_cast<double>(
                note.velocity);

        durationSum +=
            durationBeats(
                record,
                note);

        boundaries.push_back(
        {
            note.startTick,
            true
        });

        boundaries.push_back(
        {
            note.endTick,
            false
        });
    }

    std::sort(
        boundaries.begin(),
        boundaries.end(),
        [](const Boundary& left,
           const Boundary& right)
        {
            if (left.tick != right.tick)
                return left.tick < right.tick;

            // End events are processed before starts at the same tick.
            return static_cast<int>(
                       left.start) <
                   static_cast<int>(
                       right.start);
        });

    int activeNotes = 0;
    int maximumPolyphony = 0;

    for (const auto& boundary :
         boundaries)
    {
        if (boundary.start)
        {
            ++activeNotes;
            maximumPolyphony =
                std::max(
                    maximumPolyphony,
                    activeNotes);
        }
        else
        {
            activeNotes =
                std::max(
                    0,
                    activeNotes - 1);
        }
    }

    const double lengthBeats =
        static_cast<double>(
            record.lengthTicks) /
        static_cast<double>(
            record.ticksPerQuarterNote);

    if (lengthBeats <= 0.0)
        return analysis;

    analysis.notesPerBeat =
        static_cast<double>(
            noteCount) /
        lengthBeats;

    analysis.averagePitch =
        pitchSum /
        static_cast<double>(
            noteCount);

    analysis.pitchRange =
        static_cast<double>(
            maximumPitch -
            minimumPitch);

    analysis.averageVelocity =
        velocitySum /
        static_cast<double>(
            noteCount);

    analysis.averageDurationBeats =
        durationSum /
        static_cast<double>(
            noteCount);

    analysis.maxPolyphony =
        static_cast<double>(
            maximumPolyphony);

    analysis.valid =
        true;

    return analysis;
}

} // namespace midigengx::music
