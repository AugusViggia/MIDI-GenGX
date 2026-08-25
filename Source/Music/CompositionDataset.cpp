#include "CompositionDataset.h"

#include <algorithm>
#include <numeric>

namespace midigengx::music
{

bool CompositionDataset::isValid() const noexcept
{
    if (samples.empty())
        return true;

    std::string previousId;

    for (const auto& sample :
         samples)
    {
        if (!sample.isValid() ||
            sample.schemaVersion !=
                schemaVersion)
        {
            return false;
        }

        if (!previousId.empty() &&
            sample.sampleId <= previousId)
        {
            return false;
        }

        previousId =
            sample.sampleId;
    }

    return true;
}

std::size_t CompositionDataset::size() const noexcept
{
    return samples.size();
}

std::size_t CompositionDataset::sectionCount() const noexcept
{
    std::size_t total = 0;

    for (const auto& sample :
         samples)
    {
        total +=
            sample.sectionCount();
    }

    return total;
}

double CompositionDataset::averageSectionCount() const noexcept
{
    if (samples.empty())
        return 0.0;

    return static_cast<double>(
               sectionCount()) /
           static_cast<double>(
               samples.size());
}

const CompositionDatasetSample*
CompositionDataset::findById(
    const std::string& sampleId) const noexcept
{
    if (sampleId.empty())
        return nullptr;

    const auto it =
        std::lower_bound(
            samples.begin(),
            samples.end(),
            sampleId,
            [](const CompositionDatasetSample& sample,
               const std::string& id)
            {
                return sample.sampleId < id;
            });

    if (it == samples.end() ||
        it->sampleId != sampleId)
    {
        return nullptr;
    }

    return &(*it);
}

CompositionDataset buildCompositionDataset(
    const std::vector<CompositionDatasetInput>& inputs) noexcept
{
    CompositionDataset dataset;

    dataset.samples.reserve(
        inputs.size());

    for (const auto& input :
         inputs)
    {
        const auto sample =
            buildCompositionDatasetSample(
                input.snapshot,
                input.sampleId);

        if (!sample.isValid())
            continue;

        dataset.samples.push_back(
            sample);
    }

    std::sort(
        dataset.samples.begin(),
        dataset.samples.end(),
        [](const CompositionDatasetSample& a,
           const CompositionDatasetSample& b)
        {
            return a.sampleId <
                   b.sampleId;
        });

    dataset.samples.erase(
        std::unique(
            dataset.samples.begin(),
            dataset.samples.end(),
            [](const CompositionDatasetSample& a,
               const CompositionDatasetSample& b)
            {
                return a.sampleId ==
                       b.sampleId;
            }),
        dataset.samples.end());

    return dataset;
}

} // namespace midigengx::music
