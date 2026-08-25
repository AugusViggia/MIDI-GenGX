#pragma once

#include "CompositionDataset.h"

#include <cstddef>
#include <vector>

namespace midigengx::music
{

struct CompositionDatasetPartition
{
    std::vector<std::size_t> trainingIndices;
    std::vector<std::size_t> validationIndices;
    std::vector<std::size_t> testIndices;

    double validationRatio = 0.0;
    double testRatio = 0.0;

    bool analysisValid = false;

    bool isValid(
        std::size_t datasetSize) const noexcept;

    std::size_t trainingCount() const noexcept;
    std::size_t validationCount() const noexcept;
    std::size_t testCount() const noexcept;
};

CompositionDatasetPartition buildCompositionDatasetPartition(
    const CompositionDataset& dataset,
    double validationRatio,
    double testRatio) noexcept;

} // namespace midigengx::music
