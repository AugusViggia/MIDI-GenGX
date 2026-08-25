#include "CompositionConditionedTrainingDataset.h"

#include <algorithm>
#include <unordered_set>

namespace midigengx::music
{

bool CompositionConditionedTrainingDataset::isValid()
    const noexcept
{
    if (!valid ||
        !vocabulary.isValid() ||
        samples.empty())
    {
        return false;
    }

    std::unordered_set<std::string> ids;

    for (const auto& sample :
         samples)
    {
        if (!sample.isValid() ||
            !ids.insert(
                sample.sequence.sampleId).second)
        {
            return false;
        }

        if (sample.composerIndex >=
                vocabulary.composers.size() ||
            sample.styleIndex >=
                vocabulary.styles.size() ||
            sample.eraIndex >=
                vocabulary.eras.size() ||
            sample.instrumentationIndex >=
                vocabulary.instrumentations.size())
        {
            return false;
        }
    }

    return true;
}

std::size_t
CompositionConditionedTrainingDataset::sampleCount()
    const noexcept
{
    return samples.size();
}

CompositionConditionedTrainingDataset
buildCompositionConditionedTrainingDataset(
    const std::vector<CompositionMidiTrainingSequence>& sequences,
    const CompositionSequenceMetadataCatalog& metadataCatalog)
    noexcept
{
    CompositionConditionedTrainingDataset result;

    if (sequences.empty() ||
        !metadataCatalog.isValid())
    {
        return result;
    }

    std::vector<CompositionSequenceMetadata> verifiedMetadata;

    for (const auto& entry :
         metadataCatalog.entries)
    {
        if (entry.verified)
        {
            verifiedMetadata.push_back(
                entry);
        }
    }

    if (verifiedMetadata.empty())
        return result;

    result.vocabulary =
        buildCompositionConditioningVocabulary(
            verifiedMetadata);

    if (!result.vocabulary.isValid())
        return result;

    result.samples.reserve(
        sequences.size());

    for (const auto& sequence :
         sequences)
    {
        if (!sequence.isValid())
            return CompositionConditionedTrainingDataset{};

        const auto* metadata =
            metadataCatalog.findBySampleId(
                sequence.sampleId);

        if (metadata == nullptr ||
            !metadata->isValid() ||
            !metadata->verified)
        {
            return CompositionConditionedTrainingDataset{};
        }

        CompositionConditionedTrainingSample sample;

        sample.sequence =
            sequence;

        sample.metadata =
            *metadata;

        sample.composerIndex =
            result.vocabulary.composerIndex(
                metadata->composerId);

        sample.styleIndex =
            result.vocabulary.styleIndex(
                metadata->styleId);

        sample.eraIndex =
            result.vocabulary.eraIndex(
                metadata->eraId);

        sample.instrumentationIndex =
            result.vocabulary.instrumentationIndex(
                metadata->instrumentationId);

        sample.valid =
            true;

        if (!sample.isValid())
            return CompositionConditionedTrainingDataset{};

        result.samples.push_back(
            std::move(sample));
    }

    std::sort(
        result.samples.begin(),
        result.samples.end(),
        [](const auto& left,
           const auto& right)
        {
            return left.sequence.sampleId <
                   right.sequence.sampleId;
        });

    result.verified =
        std::all_of(
            result.samples.begin(),
            result.samples.end(),
            [](const auto& sample)
            {
                return sample.metadata.verified;
            });

    result.valid =
        true;

    if (!result.isValid())
        result.valid = false;

    return result;
}

} // namespace midigengx::music
