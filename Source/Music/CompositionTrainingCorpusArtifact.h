#pragma once

#include "CompositionDatasetPreparedView.h"

#include <cstdint>
#include <vector>

namespace midigengx::music
{

struct CompositionTrainingCorpusArtifact
{
    static constexpr std::uint32_t magic = 0x4D474443u; // "MGDC"
    static constexpr std::uint32_t version = 1;

    std::vector<std::uint8_t> bytes;

    bool isValid() const noexcept;
};

CompositionTrainingCorpusArtifact
serializeCompositionTrainingCorpus(
    const CompositionDatasetPreparedView& prepared) noexcept;

bool deserializeCompositionTrainingCorpus(
    const CompositionTrainingCorpusArtifact& artifact,
    CompositionDatasetPreparedView& prepared) noexcept;

} // namespace midigengx::music
