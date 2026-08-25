#include "CompositionDatasetPreparedView.h"

namespace midigengx::music
{

bool CompositionDatasetPreparedView::isValid() const noexcept
{
    return analysisValid &&
           normalizedBatch.isValid() &&
           partition.isValid(
               normalizedBatch.sampleCount) &&
           normalization.isValid() &&
           partition.trainingCount() +
                   partition.validationCount() +
                   partition.testCount() ==
               normalizedBatch.sampleCount;
}

std::size_t
CompositionDatasetPreparedView::trainingCount() const noexcept
{
    return partition.trainingCount();
}

std::size_t
CompositionDatasetPreparedView::validationCount() const noexcept
{
    return partition.validationCount();
}

std::size_t
CompositionDatasetPreparedView::testCount() const noexcept
{
    return partition.testCount();
}

CompositionDatasetPreparedView
prepareCompositionDatasetForLearning(
    const CompositionDataset& dataset,
    const CompositionDatasetQuality& quality,
    const CompositionDatasetManifest& manifest,
    const CompositionDatasetPartition& partition) noexcept
{
    CompositionDatasetPreparedView prepared;

    if (!dataset.isValid() ||
        !quality.isValid() ||
        !manifest.isValid() ||
        !partition.isValid(
            dataset.size()) ||
        quality.sampleCount !=
            dataset.size() ||
        manifest.sampleCount !=
            dataset.size())
    {
        return prepared;
    }

    if (!dataset.samples.empty() &&
        (quality.globalFeatureWidth !=
             CompositionDatasetSchema::globalFeatureCount ||
         quality.sectionFeatureWidth !=
             CompositionDatasetSchema::sectionFeatureCount))
    {
        return prepared;
    }

    const auto batch =
        buildCompositionDatasetBatch(
            dataset,
            manifest);

    if (!batch.isValid())
        return prepared;

    const auto normalization =
        fitCompositionDatasetNormalization(
            batch,
            partition);

    if (!normalization.isValid())
        return prepared;

    const auto normalizedBatch =
        applyCompositionDatasetNormalization(
            batch,
            normalization);

    if (!normalizedBatch.isValid())
        return prepared;

    prepared.analysisValid = true;
    prepared.normalizedBatch =
        normalizedBatch;
    prepared.partition =
        partition;
    prepared.normalization =
        normalization;

    return prepared;
}

} // namespace midigengx::music
