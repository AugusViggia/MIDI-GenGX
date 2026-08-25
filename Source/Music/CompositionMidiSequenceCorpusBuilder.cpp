#include "CompositionMidiSequenceCorpusBuilder.h"

#include "CompositionMidiCorpusDirectoryLoader.h"
#include "CompositionMidiHarmony.h"
#include "CompositionMidiSectionAnalyzer.h"
#include "CompositionMidiTrainingSequence.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <utility>
#include <vector>

namespace midigengx::music
{

namespace
{

CompositionMidiSectionAnalysis
buildWholePieceFallbackSections(
    const CompositionMidiCorpusRecord& record) noexcept
{
    CompositionMidiSectionAnalysis result;

    if (!record.isValid() ||
        record.ticksPerQuarterNote == 0 ||
        record.lengthTicks == 0 ||
        record.notes.empty())
    {
        return result;
    }

    const auto lengthBeats =
        static_cast<double>(
            record.lengthTicks) /
        static_cast<double>(
            record.ticksPerQuarterNote);

    if (lengthBeats < 4.0)
        return result;

    double pitchSum = 0.0;
    double velocitySum = 0.0;
    double durationSumBeats = 0.0;

    std::vector<
        std::pair<std::uint32_t, int>>
        boundaries;

    boundaries.reserve(
        record.notes.size() * 2);

    for (const auto& note :
         record.notes)
    {
        pitchSum +=
            static_cast<double>(
                note.midiNote);

        velocitySum +=
            static_cast<double>(
                note.velocity);

        durationSumBeats +=
            static_cast<double>(
                note.endTick -
                note.startTick) /
            static_cast<double>(
                record.ticksPerQuarterNote);

        boundaries.push_back(
        {
            note.startTick,
            +1
        });

        boundaries.push_back(
        {
            note.endTick,
            -1
        });
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
    int maximumPolyphony = 0;

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

            maximumPolyphony =
                std::max(
                    maximumPolyphony,
                    active);
        }
    }

    CompositionMidiSection section;

    section.sectionIndex = 0;
    section.role =
        PhraseSection::Opening;
    section.startTick = 0;
    section.endTick =
        record.lengthTicks;

    const auto noteCount =
        static_cast<double>(
            record.notes.size());

    section.notesPerBeat =
        noteCount /
        lengthBeats;

    section.averagePitch =
        pitchSum /
        noteCount;

    section.averageVelocity =
        velocitySum /
        noteCount;

    section.averageDurationBeats =
        durationSumBeats /
        noteCount;

    section.maxPolyphony =
        static_cast<double>(
            std::max(
                1,
                maximumPolyphony));

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

    section.valid = true;

    result.sections.push_back(
        section);

    result.valid =
        result.isValid(
            record.ticksPerQuarterNote);

    return result;
}

} // namespace

bool CompositionMidiSequenceCorpusBuildResult::isValid()
    const noexcept
{
    return valid &&
           artifact.isValid() &&
           sequenceCount > 0 &&
           sequenceCount +
               rejectedCount ==
               inputFileCount &&
           sequenceCount <=
               inputFileCount;
}

CompositionMidiSequenceCorpusBuildResult
buildCompositionMidiSequenceCorpusFromDirectory(
    const std::string& directoryPath,
    bool recursive)
    noexcept
{
    CompositionMidiSequenceCorpusBuildResult result;

    const auto load =
        loadCompositionMidiCorpusDirectory(
            directoryPath,
            recursive);

    if (!load.isValid())
        return result;

    result.inputFileCount =
        load.discoveredFileCount;

    // Files rejected by the low-level MIDI loader have already been accounted
    // for and must remain part of the final rejection count.
    result.rejectedCount =
        load.rejectedFileCount;

    if (load.records.empty())
        return result;

    std::vector<CompositionMidiTrainingSequence>
        sequences;

    sequences.reserve(
        load.records.size());

    std::set<std::string> acceptedIds;

    for (const auto& record :
         load.records)
    {
        // Prefer the real four-bar structural analysis when it is valid.
        // The whole-piece representation is a deterministic fallback only
        // for records where the preferred segmentation cannot be established.
        auto sections =
            analyzeCompositionMidiSections(
                record);

        if (!sections.isValid(
                record.ticksPerQuarterNote))
        {
            sections =
                buildWholePieceFallbackSections(
                    record);
        }

        if (!sections.isValid(
                record.ticksPerQuarterNote))
        {
            ++result.rejectedCount;
            continue;
        }

        // Harmony and motif analysis are deliberately outside the acceptance
        // boundary. Unknown/omitted enrichment must never discard valid MIDI.
        const auto sequence =
            buildCompositionMidiTrainingSequence(
                record,
                sections,
                CompositionMidiHarmonyAnalysis{},
                CompositionMidiMotifAnalysis{});

        if (!sequence.isValid())
        {
            ++result.rejectedCount;
            continue;
        }

        if (!acceptedIds.insert(
                sequence.sampleId).second)
        {
            ++result.rejectedCount;
            continue;
        }

        result.eventCount +=
            sequence.eventCount();

        sequences.push_back(
            sequence);
    }

    if (sequences.empty())
        return result;

    std::sort(
        sequences.begin(),
        sequences.end(),
        [](const CompositionMidiTrainingSequence& left,
           const CompositionMidiTrainingSequence& right)
        {
            return left.sampleId <
                   right.sampleId;
        });

    result.sequenceCount =
        sequences.size();

    result.artifact =
        serializeCompositionMidiTrainingSequences(
            sequences);

    if (!result.artifact.isValid())
        return result;

    result.valid =
        true;

    if (!result.isValid())
        result.valid = false;

    return result;
}

} // namespace midigengx::music
