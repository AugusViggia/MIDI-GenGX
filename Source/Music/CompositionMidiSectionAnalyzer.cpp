#include "CompositionMidiSectionAnalyzer.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace midigengx::music
{
namespace
{

constexpr std::uint32_t minimumSectionBeats = 4;

struct NoteBucket
{
    std::uint32_t startTick = 0;
    std::uint32_t endTick = 0;

    std::size_t noteCount = 0;
    double pitchSum = 0.0;
    double velocitySum = 0.0;
    double durationSumBeats = 0.0;
    int maximumPolyphony = 0;
};

bool finite(
    double value) noexcept
{
    return std::isfinite(value);
}

} // namespace

bool CompositionMidiSection::isValid(
    std::uint16_t ticksPerQuarterNote) const noexcept
{
    if (!valid ||
        endTick <= startTick ||
        ticksPerQuarterNote == 0 ||
        sectionIndex >
            static_cast<std::size_t>(
                std::numeric_limits<std::uint32_t>::max()) ||
        !finite(notesPerBeat) ||
        !finite(averagePitch) ||
        !finite(averageVelocity) ||
        !finite(averageDurationBeats) ||
        !finite(maxPolyphony))
    {
        return false;
    }

    const auto lengthBeats =
        static_cast<double>(
            endTick - startTick) /
        static_cast<double>(
            ticksPerQuarterNote);

    return lengthBeats >=
               static_cast<double>(
                   minimumSectionBeats) &&
           notesPerBeat >= 0.0 &&
           averagePitch >= 0.0 &&
           averagePitch <= 127.0 &&
           averageVelocity >= 0.0 &&
           averageVelocity <= 127.0 &&
           averageDurationBeats > 0.0 &&
           maxPolyphony >= 1.0 &&
           tension >= 0 &&
           tension <= 100;
}

bool CompositionMidiSectionAnalysis::isValid(
    std::uint16_t ticksPerQuarterNote) const noexcept
{
    if (!valid ||
        sections.empty())
    {
        return false;
    }

    for (const auto& section :
         sections)
    {
        if (!section.isValid(
                ticksPerQuarterNote))
        {
            return false;
        }
    }

    for (std::size_t index = 1;
         index < sections.size();
         ++index)
    {
        if (sections[index - 1].endTick !=
            sections[index].startTick)
        {
            return false;
        }
    }

    return true;
}

std::size_t CompositionMidiSectionAnalysis::sectionCount()
    const noexcept
{
    return sections.size();
}

CompositionMidiSectionAnalysis
analyzeCompositionMidiSections(
    const CompositionMidiCorpusRecord& record)
    noexcept
{
    CompositionMidiSectionAnalysis result;

    if (!record.isValid() ||
        record.ticksPerQuarterNote == 0 ||
        record.lengthTicks == 0)
    {
        return result;
    }

    const auto ppq =
        static_cast<std::uint32_t>(
            record.ticksPerQuarterNote);

    const auto beats =
        static_cast<double>(
            record.lengthTicks) /
        static_cast<double>(
            ppq);

    // V1 segmentation is intentionally conservative:
    // use 4-bar windows and preserve the real MIDI time domain.
    constexpr std::uint32_t barsPerSection = 4;
    constexpr std::uint32_t beatsPerBar = 4;
    constexpr std::uint32_t sectionBeats =
        barsPerSection * beatsPerBar;

    const auto sectionTicks =
        ppq * sectionBeats;

    const auto sectionCount =
        std::max<std::size_t>(
            1,
            static_cast<std::size_t>(
                std::ceil(
                    beats /
                    static_cast<double>(
                        sectionBeats))));

    result.sections.reserve(sectionCount);

    for (std::size_t index = 0;
         index < sectionCount;
         ++index)
    {
        const auto startTick =
            static_cast<std::uint32_t>(
                index * sectionTicks);

        const auto naturalEndTick =
            static_cast<std::uint32_t>(
                std::min<std::uint64_t>(
                    static_cast<std::uint64_t>(
                        record.lengthTicks),
                    static_cast<std::uint64_t>(
                        startTick) +
                    static_cast<std::uint64_t>(
                        sectionTicks)));

        if (naturalEndTick <= startTick)
            continue;

        NoteBucket bucket;
        bucket.startTick = startTick;
        bucket.endTick = naturalEndTick;

        std::vector<
            std::pair<std::uint32_t, int>>
            boundaries;

        for (const auto& note :
             record.notes)
        {
            if (note.endTick <= startTick ||
                note.startTick >= naturalEndTick)
            {
                continue;
            }

            const auto clippedStart =
                std::max(
                    note.startTick,
                    startTick);

            const auto clippedEnd =
                std::min(
                    note.endTick,
                    naturalEndTick);

            if (clippedEnd <= clippedStart)
                continue;

            ++bucket.noteCount;

            bucket.pitchSum +=
                static_cast<double>(
                    note.midiNote);

            bucket.velocitySum +=
                static_cast<double>(
                    note.velocity);

            bucket.durationSumBeats +=
                static_cast<double>(
                    clippedEnd -
                    clippedStart) /
                static_cast<double>(
                    ppq);

            boundaries.push_back(
            {
                clippedStart,
                +1
            });

            boundaries.push_back(
            {
                clippedEnd,
                -1
            });
        }

        if (bucket.noteCount == 0)
        {
            // Empty sections are not useful training samples.
            continue;
        }

        std::sort(
            boundaries.begin(),
            boundaries.end(),
            [](const auto& left,
               const auto& right)
            {
                if (left.first != right.first)
                    return left.first <
                           right.first;

                return left.second <
                       right.second;
            });

        int active = 0;

        for (const auto& boundary :
             boundaries)
        {
            if (boundary.second < 0)
            {
                active =
                    std::max(
                        0,
                        active - 1);
            }
            else
            {
                ++active;

                bucket.maximumPolyphony =
                    std::max(
                        bucket.maximumPolyphony,
                        active);
            }
        }

        const auto sectionLengthBeats =
            static_cast<double>(
                naturalEndTick -
                startTick) /
            static_cast<double>(
                ppq);

        CompositionMidiSection section;

        section.sectionIndex =
            index;

        if (index == 0)
        {
            section.role =
                PhraseSection::Opening;
        }
        else if (index + 1 ==
                 sectionCount)
        {
            section.role =
                PhraseSection::Cadence;
        }
        else
        {
            section.role =
                PhraseSection::Development;
        }

        section.startTick =
            startTick;

        section.endTick =
            naturalEndTick;

        section.notesPerBeat =
            static_cast<double>(
                bucket.noteCount) /
            sectionLengthBeats;

        section.averagePitch =
            bucket.pitchSum /
            static_cast<double>(
                bucket.noteCount);

        section.averageVelocity =
            bucket.velocitySum /
            static_cast<double>(
                bucket.noteCount);

        section.averageDurationBeats =
            bucket.durationSumBeats /
            static_cast<double>(
                bucket.noteCount);

        section.maxPolyphony =
            static_cast<double>(
                std::max(
                    1,
                    bucket.maximumPolyphony));

        const auto densityScore =
            std::clamp(
                section.notesPerBeat / 4.0,
                0.0,
                1.0);

        const auto velocityScore =
            std::clamp(
                section.averageVelocity / 127.0,
                0.0,
                1.0);

        section.tension =
            std::clamp(
                static_cast<int>(
                    std::lround(
                        (densityScore * 0.6 +
                         velocityScore * 0.4) *
                        100.0)),
                0,
                100);

        section.valid =
            true;

        result.sections.push_back(
            section);
    }

    result.valid =
        true;

    if (!result.isValid(
            record.ticksPerQuarterNote))
    {
        result.valid =
            false;
    }

    return result;
}

} // namespace midigengx::music
