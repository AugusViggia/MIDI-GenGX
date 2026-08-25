#include "CompositionDatasetBatch.h"

#include <algorithm>

namespace midigengx::music
{

bool CompositionDatasetBatch::isValid() const noexcept
{
    if (!analysisValid ||
        globalFeatureWidth !=
            CompositionDatasetSchema::globalFeatureCount ||
        sectionFeatureWidth !=
            CompositionDatasetSchema::sectionFeatureCount)
    {
        return false;
    }

    const auto expectedGlobal =
        sampleCount *
        globalFeatureWidth;

    const auto expectedSections =
        sampleCount *
        maxSectionCount;

    const auto expectedSectionValues =
        expectedSections *
        sectionFeatureWidth;

    return globalMatrix.size() ==
               expectedGlobal &&
           sectionMatrix.size() ==
               expectedSectionValues &&
           sectionMask.size() ==
               expectedSections;
}

std::size_t
CompositionDatasetBatch::globalMatrixSize() const noexcept
{
    return globalMatrix.size();
}

std::size_t
CompositionDatasetBatch::sectionMatrixSize() const noexcept
{
    return sectionMatrix.size();
}

CompositionDatasetBatch
buildCompositionDatasetBatch(
    const CompositionDataset& dataset,
    const CompositionDatasetManifest& manifest) noexcept
{
    CompositionDatasetBatch batch;

    if (!dataset.isValid() ||
        !manifest.isValid() ||
        manifest.sampleCount !=
            dataset.size() ||
        manifest.globalFeatureWidth !=
            CompositionDatasetSchema::globalFeatureCount ||
        manifest.sectionFeatureWidth !=
            CompositionDatasetSchema::sectionFeatureCount)
    {
        return batch;
    }

    batch.analysisValid = true;
    batch.sampleCount =
        dataset.size();

    if (dataset.size() == 0)
        return batch;

    batch.maxSectionCount =
        dataset.samples.front()
            .sectionCount();

    for (const auto& sample :
         dataset.samples)
    {
        batch.maxSectionCount =
            std::max(
                batch.maxSectionCount,
                sample.sectionCount());
    }

    batch.globalMatrix.reserve(
        batch.sampleCount *
        batch.globalFeatureWidth);

    batch.sectionMatrix.assign(
        batch.sampleCount *
            batch.maxSectionCount *
            batch.sectionFeatureWidth,
        0.0);

    batch.sectionMask.assign(
        batch.sampleCount *
            batch.maxSectionCount,
        0.0);

    for (std::size_t sampleIndex = 0;
         sampleIndex < batch.sampleCount;
         ++sampleIndex)
    {
        const auto& sample =
            dataset.samples[sampleIndex];

        batch.globalMatrix.insert(
            batch.globalMatrix.end(),
            sample.globalFeatures.begin(),
            sample.globalFeatures.end());

        for (std::size_t sectionIndex = 0;
             sectionIndex <
                 sample.sectionFeatures.size();
             ++sectionIndex)
        {
            const auto base =
                (sampleIndex *
                     batch.maxSectionCount +
                 sectionIndex);

            const auto sectionBase =
                base *
                batch.sectionFeatureWidth;

            const auto& section =
                sample.sectionFeatures[
                    sectionIndex];

            for (std::size_t featureIndex = 0;
                 featureIndex <
                     batch.sectionFeatureWidth;
                 ++featureIndex)
            {
                batch.sectionMatrix[
                    sectionBase +
                    featureIndex] =
                    section[featureIndex];
            }

            batch.sectionMask[base] =
                1.0;
        }
    }

    return batch;
}

} // namespace midigengx::music
