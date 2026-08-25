#pragma once

#include "CompositionDataset.h"

#include <cstddef>

namespace midigengx::music
{

struct CompositionDatasetQuality
{
    bool valid = false;

    std::size_t sampleCount = 0;
    std::size_t invalidSampleCount = 0;
    std::size_t duplicateIdCount = 0;

    std::size_t globalFeatureWidth = 0;
    std::size_t sectionFeatureWidth = 0;

    std::size_t minSectionCount = 0;
    std::size_t maxSectionCount = 0;

    bool isValid() const noexcept;
};

CompositionDatasetQuality assessCompositionDatasetQuality(
    const CompositionDataset& dataset) noexcept;

} // namespace midigengx::music
