#include "CompositionMidiTrainingSequence.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace midigengx::music
{
namespace
{

double clampUnit(
    double value) noexcept
{
    return std::clamp(
        value,
        0.0,
        1.0);
}

double normalizeSigned(
    double value,
    double magnitude) noexcept
{
    if (magnitude <= 0.0)
        return 0.0;

    return std::clamp(
        value / magnitude,
        -1.0,
        1.0);
}

double encodeRole(
    PhraseSection role) noexcept
{
    switch (role)
    {
        case PhraseSection::Opening:
            return 0.0;

        case PhraseSection::Development:
            return 1.0 / 3.0;

        case PhraseSection::Preparation:
            return 2.0 / 3.0;

        case PhraseSection::Cadence:
            return 1.0;
    }

    return 0.0;
}

double encodeHarmonyQuality(
    ChordQuality quality) noexcept
{
    switch (quality)
    {
        case ChordQuality::Major:
            return 0.0;

        case ChordQuality::Minor:
            return 0.2;

        case ChordQuality::Diminished:
            return 0.4;

        case ChordQuality::Augmented:
            return 0.6;

        case ChordQuality::Suspended:
            return 0.8;

        case ChordQuality::Unknown:
            return 1.0;
    }

    return 1.0;
}

std::size_t findSectionIndex(
    const CompositionMidiSectionAnalysis& sections,
    std::uint32_t tick) noexcept
{
    for (std::size_t index = 0;
         index < sections.sections.size();
         ++index)
    {
        const auto& section =
            sections.sections[index];

        if (tick >= section.startTick &&
            tick < section.endTick)
        {
            return index;
        }
    }

    if (!sections.sections.empty() &&
        tick >= sections.sections.back().endTick)
    {
        return sections.sections.size() - 1;
    }

    return std::numeric_limits<std::size_t>::max();
}

double sectionProgress(
    const CompositionMidiSection& section,
    std::uint32_t tick) noexcept
{
    if (section.endTick <= section.startTick)
        return 0.0;

    return clampUnit(
        static_cast<double>(
            tick - section.startTick) /
        static_cast<double>(
            section.endTick -
            section.startTick));
}

} // namespace

bool CompositionMidiTrainingEvent::isValid()
    const noexcept
{
    if (features.size() != featureCount)
        return false;

    for (const auto value :
         features)
    {
        if (!std::isfinite(value) ||
            value < -1.0 ||
            value > 1.0)
        {
            return false;
        }
    }

    return true;
}

bool CompositionMidiTrainingSequence::isValid()
    const noexcept
{
    if (!analysisValid ||
        sampleId.empty() ||
        featureWidth !=
            CompositionMidiTrainingEvent::featureCount ||
        events.empty())
    {
        return false;
    }

    for (const auto& event :
         events)
    {
        if (!event.isValid())
            return false;
    }

    return true;
}

std::size_t
CompositionMidiTrainingSequence::eventCount()
    const noexcept
{
    return events.size();
}

CompositionMidiTrainingSequence
buildCompositionMidiTrainingSequence(
    const CompositionMidiCorpusRecord& record,
    const CompositionMidiSectionAnalysis& sections,
    const CompositionMidiHarmonyAnalysis& harmony,
    const CompositionMidiMotifAnalysis& motifs)
    noexcept
{
    CompositionMidiTrainingSequence sequence;

    if (!record.isValid() ||
        !sections.isValid(
            record.ticksPerQuarterNote) ||
        record.notes.empty() ||
        record.ticksPerQuarterNote == 0)
    {
        return sequence;
    }

    sequence.sampleId =
        record.sampleId;

    sequence.events.reserve(
        record.notes.size());

    int previousPitch =
        -1;

    std::uint32_t previousStartTick =
        0;

    for (const auto& note :
         record.notes)
    {
        const auto sectionIndex =
            findSectionIndex(
                sections,
                note.startTick);

        if (sectionIndex >=
            sections.sections.size())
        {
            return CompositionMidiTrainingSequence{};
        }

        const auto& section =
            sections.sections[
                sectionIndex];

        CompositionMidiSectionHarmony fallbackHarmony;

        const auto* harmonic =
            harmony.isValid() &&
            sectionIndex <
                harmony.sections.size()
                ? &harmony.sections[
                    sectionIndex]
                : &fallbackHarmony;

        const auto deltaTicks =
            previousPitch < 0
                ? note.startTick
                : note.startTick -
                  previousStartTick;

        const auto interval =
            previousPitch < 0
                ? 0
                : static_cast<int>(
                      note.midiNote) -
                  previousPitch;

        const auto startBeat =
            static_cast<double>(
                note.startTick) /
            static_cast<double>(
                record.ticksPerQuarterNote);

        const auto durationBeats =
            static_cast<double>(
                note.endTick -
                note.startTick) /
            static_cast<double>(
                record.ticksPerQuarterNote);

        const auto barPosition =
            std::fmod(
                startBeat,
                4.0) /
            4.0;

        const auto beatPosition =
            std::fmod(
                startBeat,
                1.0);

        const auto pitchClass =
            static_cast<int>(
                note.midiNote % 12);

        const auto octave =
            static_cast<int>(
                note.midiNote / 12);

        CompositionMidiTrainingEvent event;

        event.features =
        {
            // 0 — absolute pitch
            clampUnit(
                static_cast<double>(
                    note.midiNote) /
                127.0),

            // 1 — velocity
            clampUnit(
                static_cast<double>(
                    note.velocity) /
                127.0),

            // 2 — duration in beats
            clampUnit(
                durationBeats /
                8.0),

            // 3 — delta from previous onset
            clampUnit(
                static_cast<double>(
                    deltaTicks) /
                static_cast<double>(
                    record.ticksPerQuarterNote *
                    4)),

            // 4 — position inside four-beat bar
            clampUnit(
                barPosition),

            // 5 — position inside beat
            clampUnit(
                beatPosition),

            // 6 — pitch class
            clampUnit(
                static_cast<double>(
                    pitchClass) /
                11.0),

            // 7 — register/octave
            clampUnit(
                static_cast<double>(
                    octave) /
                10.0),

            // 8 — melodic interval
            normalizeSigned(
                static_cast<double>(
                    interval),
                12.0),

            // 9 — duration / beat ratio
            clampUnit(
                durationBeats),

            // 10 — structural section progress
            sectionProgress(
                section,
                note.startTick),

            // 11 — section role
            encodeRole(
                section.role),

            // 12 — section tension
            clampUnit(
                static_cast<double>(
                    section.tension) /
                100.0),

            // 13 — section tension delta
            normalizeSigned(
                sectionIndex == 0
                    ? 0.0
                    : static_cast<double>(
                          section.tension -
                          sections.sections[
                              sectionIndex - 1].tension),
                100.0),

            // 14 — harmonic scale degree
            harmonic->valid
                ? clampUnit(
                    static_cast<double>(
                        harmonic->scaleDegree) /
                    6.0)
                : 0.0,

            // 15 — chord quality
            harmonic->valid
                ? encodeHarmonyQuality(
                    harmonic->quality)
                : 1.0,

            // 16 — harmonic degree delta
            harmonic->valid
                ? normalizeSigned(
                    static_cast<double>(
                        harmonic->scaleDegree),
                    6.0)
                : 0.0,

            // 17 — harmonic confidence
            harmonic->valid
                ? clampUnit(
                    harmonic->confidence)
                : 0.0,

            // 18 — normalized sequence progress
            record.lengthTicks == 0
                ? 0.0
                : clampUnit(
                    static_cast<double>(
                        note.startTick) /
                    static_cast<double>(
                        record.lengthTicks)),

            // 19 — channel identity
            clampUnit(
                static_cast<double>(
                    note.channel) /
                15.0)
        };

        if (!event.isValid())
            return CompositionMidiTrainingSequence{};

        sequence.events.push_back(
            std::move(event));

        previousPitch =
            static_cast<int>(
                note.midiNote);

        previousStartTick =
            note.startTick;
    }

    sequence.analysisValid =
        true;

    return sequence;
}

} // namespace midigengx::music
