#pragma once

#include "CompositionDatasetManifest.h"

#include <cstddef>
#include <vector>

namespace midigengx::music
{

struct CompositionDatasetBatch
{
    static constexpr int batchVersion = 1;

    std::size_t sampleCount = 0;
    std::size_t maxSectionCount = 0;

    std::size_t globalFeatureWidth =
        CompositionDatasetSchema::globalFeatureCount;

    std::size_t sectionFeatureWidth =
        CompositionDatasetSchema::sectionFeatureCount;

    // Row-major flattened representation:
    // [sample][feature].
    std::vector<double> globalMatrix;

    // Row-major flattened representation:
    // [sample][section][feature].
    std::vector<double> sectionMatrix;

    // One mask value per [sample][section].
    // 1.0 = real section, 0.0 = padding.
    std::vector<double> sectionMask;

    bool analysisValid = false;

    bool isValid() const noexcept;

    std::size_t globalMatrixSize() const noexcept;
    std::size_t sectionMatrixSize() const noexcept;
};

CompositionDatasetBatch buildCompositionDatasetBatch(
    const CompositionDataset& dataset,
    const CompositionDatasetManifest& manifest) noexcept;

} // namespace midigengx::music
