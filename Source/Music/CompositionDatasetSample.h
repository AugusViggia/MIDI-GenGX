#pragma once

#include "CompositionKnowledgeSnapshot.h"

#include <cstddef>
#include <string>
#include <vector>

namespace midigengx::music
{

struct CompositionDatasetSample
{
    static constexpr int schemaVersion = 1;

    std::string sampleId;

    // Stable, ordered numeric representation for future ML/data pipelines.
    std::vector<double> globalFeatures;

    // One fixed-width feature vector per composition section.
    std::vector<std::vector<double>> sectionFeatures;

    bool analysisValid = false;

    bool isValid() const noexcept;

    std::size_t sectionCount() const noexcept;
};

CompositionDatasetSample buildCompositionDatasetSample(
    const CompositionKnowledgeSnapshot& snapshot,
    const std::string& sampleId) noexcept;

} // namespace midigengx::music
