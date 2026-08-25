#include "CompositionSequenceMetadataCatalog.h"

#include <algorithm>
#include <unordered_set>

namespace midigengx::music
{

bool CompositionSequenceMetadataCatalog::isValid()
    const noexcept
{
    if (!valid ||
        entries.empty())
    {
        return false;
    }

    std::unordered_set<std::string> ids;

    for (const auto& entry :
         entries)
    {
        if (!entry.isValid() ||
            !ids.insert(
                entry.sampleId).second)
        {
            return false;
        }
    }

    return true;
}

const CompositionSequenceMetadata*
CompositionSequenceMetadataCatalog::findBySampleId(
    const std::string& sampleId)
    const noexcept
{
    for (const auto& entry :
         entries)
    {
        if (entry.sampleId ==
            sampleId)
        {
            return &entry;
        }
    }

    return nullptr;
}

CompositionSequenceMetadataCatalog
buildCompositionSequenceMetadataCatalog(
    const std::vector<CompositionSequenceMetadata>& metadata)
    noexcept
{
    CompositionSequenceMetadataCatalog result;

    if (metadata.empty())
        return result;

    result.entries =
        metadata;

    std::sort(
        result.entries.begin(),
        result.entries.end(),
        [](const auto& left,
           const auto& right)
        {
            return left.sampleId <
                   right.sampleId;
        });

    result.verified =
        std::all_of(
            result.entries.begin(),
            result.entries.end(),
            [](const auto& entry)
            {
                return entry.verified;
            });

    result.valid =
        true;

    if (!result.isValid())
        result.valid = false;

    return result;
}

} // namespace midigengx::music
