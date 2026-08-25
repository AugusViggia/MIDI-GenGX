#pragma once

#include "CompositionConditionedTrainingDataset.h"

#include <cstdint>
#include <vector>

namespace midigengx::music
{

// Deterministic conditioning manifest. The canonical note sequences remain
// in CompositionMidiTrainingCorpusArtifact and are joined by sampleId.
struct CompositionConditionedTrainingDatasetArtifact
{
    static constexpr std::uint32_t magic = 0x4D474344u; // "MGCD"
    static constexpr std::uint32_t version = 1;

    std::vector<std::uint8_t> bytes;

    bool isValid() const noexcept;
};

CompositionConditionedTrainingDatasetArtifact
serializeCompositionConditionedTrainingDataset(
    const CompositionConditionedTrainingDataset& dataset) noexcept;

bool deserializeCompositionConditionedTrainingDataset(
    const CompositionConditionedTrainingDatasetArtifact& artifact,
    CompositionConditionedTrainingDataset& dataset) noexcept;

} // namespace midigengx::music
