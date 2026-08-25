#include "CompositionMidiDatasetBuilder.h"

#include "CompositionMidiCorpusAnalysis.h"
#include "CompositionMidiDatasetFeatureExtractor.h"
#include "CompositionMidiHarmony.h"
#include "CompositionMidiMotifAnalysis.h"
#include "CompositionMidiSectionAnalyzer.h"

#include <algorithm>
#include <set>
#include <string>

namespace midigengx::music
{
namespace
{

bool appendUniqueSample(
    CompositionDataset& dataset,
    std::set<std::string>& acceptedIds,
    CompositionDatasetSample sample) noexcept
{
    if (!sample.isValid())
        return false;

    const auto inserted =
        acceptedIds.insert(
            sample.sampleId);

    if (!inserted.second)
        return false;

    dataset.samples.push_back(
        std::move(sample));

    return true;
}

} // namespace

bool CompositionMidiDatasetBuildResult::isValid()
    const noexcept
{
    return valid &&
           dataset.isValid() &&
           acceptedCount ==
               dataset.size() &&
           rejectedCount +
               acceptedCount ==
               inputCount;
}

CompositionMidiDatasetBuildResult
buildCompositionDatasetFromMidiCorpus(
    const std::vector<CompositionMidiCorpusRecord>& records)
    noexcept
{
    CompositionMidiDatasetBuildResult result;

    result.inputCount =
        records.size();

    result.dataset.samples.reserve(
        records.size());

    CompositionMidiDatasetFeatureExtractor extractor;

    // The dataset vector is sorted only after all samples have been built.
    // Therefore duplicate detection cannot use CompositionDataset::findById(),
    // which relies on sorted storage. Keep an independent ordered ID set while
    // ingesting the corpus.
    std::set<std::string> acceptedIds;

    for (const auto& record :
         records)
    {
        if (!record.isValid())
        {
            ++result.rejectedCount;
            continue;
        }

        const auto analysis =
            analyzeCompositionMidiCorpus(
                record);

        if (!analysis.isValid())
        {
            ++result.rejectedCount;
            continue;
        }

        const auto sections =
            analyzeCompositionMidiSections(
                record);

        if (!sections.isValid(
                record.ticksPerQuarterNote))
        {
            ++result.rejectedCount;
            continue;
        }

        const auto harmony =
            analyzeCompositionMidiHarmony(
                record,
                sections);

        if (!harmony.isValid())
        {
            ++result.rejectedCount;
            continue;
        }

        const auto motifs =
            analyzeCompositionMidiMotifs(
                record,
                sections);

        if (!motifs.isValid())
        {
            ++result.rejectedCount;
            continue;
        }

        const auto sample =
            extractor.buildSample(
                record,
                analysis,
                sections,
                harmony,
                motifs,
                record.sampleId);

        if (appendUniqueSample(
                result.dataset,
                acceptedIds,
                sample))
        {
            ++result.acceptedCount;
        }
        else
        {
            ++result.rejectedCount;
        }
    }

    std::sort(
        result.dataset.samples.begin(),
        result.dataset.samples.end(),
        [](const CompositionDatasetSample& left,
           const CompositionDatasetSample& right)
        {
            return left.sampleId <
                   right.sampleId;
        });

    result.valid =
        true;

    if (!result.isValid())
        result.valid = false;

    return result;
}

} // namespace midigengx::music
