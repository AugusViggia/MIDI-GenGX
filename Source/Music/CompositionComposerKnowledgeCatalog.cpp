#include "CompositionComposerKnowledgeCatalog.h"

#include <algorithm>
#include <unordered_map>

namespace midigengx::music
{

bool CompositionComposerKnowledgeGroup::isValid()
    const noexcept
{
    if (composerId.empty() ||
        samples.empty())
    {
        return false;
    }

    for (const auto& sample :
         samples)
    {
        if (!sample.isValid() ||
            sample.metadata.composerId !=
                composerId)
        {
            return false;
        }
    }

    return true;
}

std::size_t
CompositionComposerKnowledgeGroup::sampleCount()
    const noexcept
{
    return samples.size();
}

bool CompositionComposerKnowledgeCatalog::isValid()
    const noexcept
{
    if (!valid ||
        composers.empty())
    {
        return false;
    }

    std::size_t total = 0;

    for (std::size_t index = 0;
         index < composers.size();
         ++index)
    {
        const auto& group =
            composers[index];

        if (!group.isValid() ||
            (index > 0 &&
             composers[index - 1].composerId >=
                 group.composerId))
        {
            return false;
        }

        total +=
            group.samples.size();
    }

    return total > 0;
}

std::size_t
CompositionComposerKnowledgeCatalog::composerCount()
    const noexcept
{
    return composers.size();
}

std::size_t
CompositionComposerKnowledgeCatalog::sampleCount()
    const noexcept
{
    std::size_t total = 0;

    for (const auto& group :
         composers)
    {
        total +=
            group.samples.size();
    }

    return total;
}

const CompositionComposerKnowledgeGroup*
CompositionComposerKnowledgeCatalog::findComposer(
    const std::string& composerId)
    const noexcept
{
    const auto iterator =
        std::lower_bound(
            composers.begin(),
            composers.end(),
            composerId,
            [](const auto& group,
               const std::string& value)
            {
                return group.composerId <
                       value;
            });

    if (iterator == composers.end() ||
        iterator->composerId != composerId)
    {
        return nullptr;
    }

    return &*iterator;
}

CompositionComposerKnowledgeCatalog
buildCompositionComposerKnowledgeCatalog(
    const std::vector<CompositionComposerKnowledgeSample>& samples,
    bool requireVerifiedMetadata)
    noexcept
{
    CompositionComposerKnowledgeCatalog result;

    if (samples.empty())
        return result;

    std::unordered_map<std::string, std::size_t> composerIndices;
    composerIndices.reserve(samples.size());

    for (const auto& sample :
         samples)
    {
        if (!sample.isValid() ||
            sample.metadata.composerId.empty() ||
            (requireVerifiedMetadata &&
             !sample.metadata.verified))
        {
            return result;
        }

        const auto [iterator, inserted] =
            composerIndices.emplace(
                sample.metadata.composerId,
                result.composers.size());

        if (inserted)
        {
            CompositionComposerKnowledgeGroup group;

            group.composerId =
                sample.metadata.composerId;

            group.samples.push_back(
                sample);

            result.composers.push_back(
                std::move(group));
        }
        else
        {
            result.composers[
                iterator->second].samples.push_back(
                sample);
        }
    }

    std::sort(
        result.composers.begin(),
        result.composers.end(),
        [](const auto& left,
           const auto& right)
        {
            return left.composerId <
                   right.composerId;
        });

    for (auto& group :
         result.composers)
    {
        std::sort(
            group.samples.begin(),
            group.samples.end(),
            [](const auto& left,
               const auto& right)
            {
                return left.composition.sampleId <
                       right.composition.sampleId;
            });
    }

    result.verified =
        std::all_of(
            samples.begin(),
            samples.end(),
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
