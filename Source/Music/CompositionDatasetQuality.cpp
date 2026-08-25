#include "CompositionDatasetQuality.h"

#include <string>

namespace midigengx::music
{

bool CompositionDatasetQuality::isValid() const noexcept
{
    if (!valid)
        return false;

    if (invalidSampleCount != 0 ||
        duplicateIdCount != 0)
    {
        return false;
    }

    if (sampleCount == 0)
    {
        return globalFeatureWidth == 0 &&
               sectionFeatureWidth == 0 &&
               minSectionCount == 0 &&
               maxSectionCount == 0;
    }

    return globalFeatureWidth > 0 &&
           sectionFeatureWidth > 0 &&
           minSectionCount > 0 &&
           maxSectionCount >= minSectionCount;
}

CompositionDatasetQuality
assessCompositionDatasetQuality(
    const CompositionDataset& dataset) noexcept
{
    CompositionDatasetQuality quality;

    quality.valid = true;
    quality.sampleCount =
        dataset.samples.size();

    if (!dataset.isValid())
    {
        quality.valid = false;
        quality.invalidSampleCount =
            dataset.samples.size();

        return quality;
    }

    if (dataset.samples.empty())
        return quality;

    quality.globalFeatureWidth =
        dataset.samples.front()
            .globalFeatures.size();

    quality.sectionFeatureWidth =
        dataset.samples.front()
            .sectionFeatures.front()
            .size();

    quality.minSectionCount =
        dataset.samples.front()
            .sectionCount();

    quality.maxSectionCount =
        quality.minSectionCount;

    std::string previousId;

    for (const auto& sample :
         dataset.samples)
    {
        if (!sample.isValid())
        {
            ++quality.invalidSampleCount;
            continue;
        }

        if (!previousId.empty() &&
            sample.sampleId ==
                previousId)
        {
            ++quality.duplicateIdCount;
        }

        previousId =
            sample.sampleId;

        if (sample.globalFeatures.size() !=
            quality.globalFeatureWidth)
        {
            quality.valid = false;
        }

        if (sample.sectionCount() == 0)
        {
            quality.valid = false;
            continue;
        }

        for (const auto& row :
             sample.sectionFeatures)
        {
            if (row.size() !=
                quality.sectionFeatureWidth)
            {
                quality.valid = false;
            }
        }

        quality.minSectionCount =
            std::min(
                quality.minSectionCount,
                sample.sectionCount());

        quality.maxSectionCount =
            std::max(
                quality.maxSectionCount,
                sample.sectionCount());
    }

    if (quality.invalidSampleCount != 0 ||
        quality.duplicateIdCount != 0)
    {
        quality.valid = false;
    }

    return quality;
}

} // namespace midigengx::music
