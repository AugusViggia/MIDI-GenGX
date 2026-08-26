#pragma once

#include "CompositionKnowledgeTrainingDataset.h"

#include <cstdint>
#include <vector>

namespace midigengx::music
{

struct CompositionKnowledgeTrainingDatasetArtifact
{
    static constexpr std::uint32_t magic = 0x4D474B44u; // "MGKD"
    static constexpr std::uint32_t version = 1;

    std::vector<std::uint8_t> bytes;

    bool isValid() const noexcept;
};

CompositionKnowledgeTrainingDatasetArtifact
serializeCompositionKnowledgeTrainingDataset(
    const CompositionKnowledgeTrainingDataset& dataset) noexcept;

} // namespace midigengx::music
