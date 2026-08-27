#include "CompositionRealComposerCorpusPreparation.h"

#include "CompositionMidiCorpusDirectoryLoader.h"
#include "CompositionMidiHarmony.h"
#include "CompositionMidiMotifAnalysis.h"
#include "CompositionMidiSectionAnalyzer.h"

#include <algorithm>
#include <set>

namespace midigengx::music
{

bool CompositionRealComposerCorpusPreparationResult::isValid()
    const noexcept
{
    return valid &&
           inputSampleCount ==
               acceptedSampleCount +
               rejectedSampleCount &&
           acceptedSampleCount ==
               sequences.size() &&
           !sequences.empty() &&
           metadataCatalog.isValid() &&
           conditionedDataset.isValid() &&
           conditionedDataset.sampleCount() ==
               sequences.size();
}

CompositionRealComposerCorpusPreparationResult
prepareRealComposerCorpusFromRecords(
    const std::vector<CompositionMidiCorpusRecord>& records,
    const CompositionSequenceMetadataCatalog& metadataCatalog)
    noexcept
{
    CompositionRealComposerCorpusPreparationResult result;

    if (records.empty() ||
        !metadataCatalog.isValid())
    {
        return result;
    }

    result.inputSampleCount =
        records.size();

    result.metadataCatalog =
        metadataCatalog;

    std::set<std::string> acceptedIds;

    for (const auto& record :
         records)
    {
        if (!record.isValid())
        {
            ++result.rejectedSampleCount;
            continue;
        }

        const auto* metadata =
            metadataCatalog.findBySampleId(
                record.sampleId);

        if (metadata == nullptr ||
            !metadata->isValid() ||
            !metadata->verified)
        {
            ++result.rejectedSampleCount;
            continue;
        }

        auto sections =
            analyzeCompositionMidiSections(
                record);

        if (!sections.isValid(
                record.ticksPerQuarterNote))
        {
            ++result.rejectedSampleCount;
            continue;
        }

        const auto harmony =
            analyzeCompositionMidiHarmony(
                record,
                sections);

        const auto motifs =
            analyzeCompositionMidiMotifs(
                record,
                sections);

        // A globally valid key and a section-aligned harmony analysis are
        // required. An individual section may legitimately have Unknown
        // harmony; the training representation has explicit fallback values
        // for that uncertainty and should not discard the whole composition.
        //
        // Motif analysis is deliberately computed and passed through the
        // sequence-building boundary, but its current 20D representation does
        // not yet encode motif-family features. Therefore a motif-analysis
        // failure must not discard an otherwise valid musical sample. This
        // keeps the real-corpus gate aligned with the information the model
        // actually consumes today.
        if (!harmony.key.isValid() ||
            harmony.sections.size() !=
                sections.sections.size())
        {
            ++result.rejectedSampleCount;
            continue;
        }

        const auto sequence =
            buildCompositionMidiTrainingSequence(
                record,
                sections,
                harmony,
                motifs);

        if (!sequence.isValid() ||
            !acceptedIds.insert(
                sequence.sampleId).second)
        {
            ++result.rejectedSampleCount;
            continue;
        }

        result.sequences.push_back(
            sequence);
        ++result.acceptedSampleCount;
    }

    if (result.sequences.empty())
        return result;

    std::sort(
        result.sequences.begin(),
        result.sequences.end(),
        [](const auto& left,
           const auto& right)
        {
            return left.sampleId <
                   right.sampleId;
        });

    result.conditionedDataset =
        buildCompositionConditionedTrainingDataset(
            result.sequences,
            result.metadataCatalog);

    if (!result.conditionedDataset.isValid())
        return result;

    result.valid = true;

    if (!result.isValid())
        result.valid = false;

    return result;
}

CompositionRealComposerCorpusPreparationResult
prepareRealComposerCorpusFromDirectory(
    const std::string& directoryPath,
    bool recursive,
    const CompositionSequenceMetadataCatalog& metadataCatalog)
    noexcept
{
    const auto loaded =
        loadCompositionMidiCorpusDirectory(
            directoryPath,
            recursive);

    CompositionRealComposerCorpusPreparationResult result;

    if (!loaded.isValid())
    {
        result.inputSampleCount =
            loaded.discoveredFileCount;

        result.rejectedSampleCount =
            loaded.rejectedFileCount +
            loaded.acceptedFileCount;

        return result;
    }

    result =
        prepareRealComposerCorpusFromRecords(
            loaded.records,
            metadataCatalog);

    result.inputSampleCount =
        loaded.discoveredFileCount;

    result.rejectedSampleCount +=
        loaded.rejectedFileCount;

    return result;
}

} // namespace midigengx::music
