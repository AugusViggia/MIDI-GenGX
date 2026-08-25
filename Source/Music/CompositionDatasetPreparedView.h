#pragma once

#include "CompositionDatasetNormalization.h"

namespace midigengx::music
{

struct CompositionDatasetPreparedView
{
    static constexpr int version = 1;

    CompositionDatasetBatch normalizedBatch;
    CompositionDatasetPartition partition;
    CompositionDatasetNormalization normalization;

    bool analysisValid = false;

    bool isValid() const noexcept;

    std::size_t trainingCount() const noexcept;
    std::size_t validationCount() const noexcept;
    std::size_t testCount() const noexcept;
};

CompositionDatasetPreparedView prepareCompositionDatasetForLearning(
    const CompositionDataset& dataset,
    const CompositionDatasetQuality& quality,
    const CompositionDatasetManifest& manifest,
    const CompositionDatasetPartition& partition) noexcept;

} // namespace midigengx::music
