#pragma once

#include "CompositionDatasetBatch.h"
#include "CompositionDatasetPartition.h"

#include <cstddef>
#include <vector>

namespace midigengx::music
{

struct CompositionDatasetNormalization
{
    static constexpr int version = 1;

    std::vector<double> globalMean;
    std::vector<double> globalStdDev;

    std::vector<double> sectionMean;
    std::vector<double> sectionStdDev;

    std::size_t globalFeatureWidth =
        CompositionDatasetSchema::globalFeatureCount;

    std::size_t sectionFeatureWidth =
        CompositionDatasetSchema::sectionFeatureCount;

    bool analysisValid = false;

    bool isValid() const noexcept;
};

CompositionDatasetNormalization
fitCompositionDatasetNormalization(
    const CompositionDatasetBatch& batch,
    const CompositionDatasetPartition& partition) noexcept;

CompositionDatasetBatch applyCompositionDatasetNormalization(
    const CompositionDatasetBatch& batch,
    const CompositionDatasetNormalization& normalization) noexcept;

} // namespace midigengx::music
