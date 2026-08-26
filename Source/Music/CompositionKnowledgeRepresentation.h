#pragma once

#include "CompositionDatasetSample.h"

#include <cstddef>
#include <string>

namespace midigengx::music
{

// Canonical Phase 115 representation shared by analysis and future
// knowledge-conditioning layers. It composes the existing semantic
// composition snapshot with the existing fixed-width numeric dataset
// projection; it does not introduce a second feature extraction pipeline.
struct CompositionKnowledgeRepresentation
{
    static constexpr int version = 1;

    CompositionKnowledgeSnapshot snapshot;
    CompositionDatasetSample datasetSample;

    bool analysisValid = false;

    bool isValid() const noexcept;

    std::size_t sectionCount() const noexcept;
    std::size_t globalFeatureWidth() const noexcept;
    std::size_t sectionFeatureWidth() const noexcept;
};

CompositionKnowledgeRepresentation
buildCompositionKnowledgeRepresentation(
    const CompositionKnowledgeSnapshot& snapshot,
    const std::string& sampleId) noexcept;

} // namespace midigengx::music
