#pragma once

#include "CompositionDatasetPartition.h"
#include "CompositionDatasetQuality.h"
#include "CompositionDatasetSchema.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace midigengx::music
{

struct CompositionDatasetManifest
{
    static constexpr int manifestVersion = 1;

    int schemaVersion = 0;
    std::size_t sampleCount = 0;
    std::size_t globalFeatureWidth = 0;
    std::size_t sectionFeatureWidth = 0;

    std::size_t trainingCount = 0;
    std::size_t validationCount = 0;
    std::size_t testCount = 0;

    std::uint64_t datasetSignature = 0;

    bool analysisValid = false;

    bool isValid() const noexcept;

    std::string signatureHex() const;
};

CompositionDatasetManifest buildCompositionDatasetManifest(
    const CompositionDataset& dataset,
    const CompositionDatasetQuality& quality,
    const CompositionDatasetPartition& partition) noexcept;

} // namespace midigengx::music
