#pragma once

#include "CompositionDatasetSample.h"

#include <cstddef>
#include <string>
#include <vector>

namespace midigengx::music
{

struct CompositionDataset
{
    static constexpr int schemaVersion = 1;

    std::vector<CompositionDatasetSample> samples;

    bool isValid() const noexcept;

    std::size_t size() const noexcept;

    std::size_t sectionCount() const noexcept;

    double averageSectionCount() const noexcept;

    const CompositionDatasetSample* findById(
        const std::string& sampleId) const noexcept;
};

struct CompositionDatasetInput
{
    std::string sampleId;
    CompositionKnowledgeSnapshot snapshot;
};

CompositionDataset buildCompositionDataset(
    const std::vector<CompositionDatasetInput>& inputs) noexcept;

} // namespace midigengx::music
