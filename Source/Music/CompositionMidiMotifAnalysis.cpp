#include "CompositionMidiMotifAnalysis.h"

#include <algorithm>
#include <vector>

namespace midigengx::music
{
namespace
{

Motif extractSectionMotif(
    const CompositionMidiCorpusRecord& record,
    const CompositionMidiSection& section) noexcept
{
    Motif motif;

    if (!record.isValid() ||
        section.endTick <= section.startTick ||
        record.ticksPerQuarterNote == 0)
    {
        return motif;
    }

    const auto ppq =
        static_cast<double>(
            record.ticksPerQuarterNote);

    int originPitch = -1;
    double latestEndBeat = 0.0;

    for (const auto& note :
         record.notes)
    {
        if (note.endTick <= section.startTick ||
            note.startTick >= section.endTick)
        {
            continue;
        }

        const auto clippedStart =
            std::max(
                note.startTick,
                section.startTick);

        const auto clippedEnd =
            std::min(
                note.endTick,
                section.endTick);

        if (clippedEnd <= clippedStart)
            continue;

        if (originPitch < 0)
            originPitch =
                static_cast<int>(
                    note.midiNote);

        MotifNote motifNote;

        motifNote.startBeat =
            static_cast<double>(
                clippedStart -
                section.startTick) /
            ppq;

        motifNote.durationBeats =
            static_cast<double>(
                clippedEnd -
                clippedStart) /
            ppq;

        motifNote.relativePitch =
            static_cast<int>(
                note.midiNote) -
            originPitch;

        motifNote.velocityDelta =
            static_cast<int>(
                note.velocity);

        motif.notes.push_back(
            motifNote);

        latestEndBeat =
            std::max(
                latestEndBeat,
                motifNote.startBeat +
                    motifNote.durationBeats);
    }

    if (motif.notes.empty() ||
        latestEndBeat <= 0.0)
    {
        return {};
    }

    const auto firstVelocity =
        motif.notes.front().velocityDelta;

    for (auto& note :
         motif.notes)
    {
        note.velocityDelta -=
            firstVelocity;
    }

    motif.lengthBeats =
        latestEndBeat;

    return motif;
}

} // namespace

bool CompositionMidiMotifAnalysis::isValid()
    const noexcept
{
    return valid &&
           recurrence.isValid() &&
           catalog.isValid();
}

std::size_t CompositionMidiMotifAnalysis::totalFamilyCount()
    const noexcept
{
    return catalog.size();
}

std::size_t CompositionMidiMotifAnalysis::recurringFamilyCount()
    const noexcept
{
    return catalog.recurringCount();
}

double CompositionMidiMotifAnalysis::averageOccurrenceCount()
    const noexcept
{
    return catalog.averageOccurrenceCount();
}

CompositionMidiMotifAnalysis
analyzeCompositionMidiMotifs(
    const CompositionMidiCorpusRecord& record,
    const CompositionMidiSectionAnalysis& sections)
    noexcept
{
    CompositionMidiMotifAnalysis result;

    if (!record.isValid() ||
        !sections.isValid(
            record.ticksPerQuarterNote))
    {
        return result;
    }

    std::vector<Motif> motifs;
    std::vector<std::size_t> sourcePhraseIndices;

    motifs.reserve(
        sections.sections.size());

    sourcePhraseIndices.reserve(
        sections.sections.size());

    for (const auto& section :
         sections.sections)
    {
        const auto motif =
            extractSectionMotif(
                record,
                section);

        if (!motif.isValid())
            continue;

        motifs.push_back(
            motif);

        sourcePhraseIndices.push_back(
            section.sectionIndex);
    }

    if (motifs.empty())
        return result;

    const auto graph =
        buildMotifOccurrenceGraph(
            motifs,
            sourcePhraseIndices);

    if (!graph.isValid())
        return result;

    result.recurrence =
        analyzeMotifRecurrence(
            graph);

    if (!result.recurrence.isValid())
        return result;

    result.catalog =
        buildMotifKnowledgeCatalog(
            result.recurrence);

    if (!result.catalog.isValid())
        return result;

    result.valid =
        true;

    if (!result.isValid())
        result.valid = false;

    return result;
}

} // namespace midigengx::music
