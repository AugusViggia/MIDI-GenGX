#pragma once

#include "CompositionNeuralModel.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace midigengx::music
{

struct CompositionNeuralModelArtifact
{
    static constexpr std::uint32_t version = 1;
    static constexpr std::uint32_t magic = 0x4D474E58u; // "MGNX"

    std::vector<std::uint8_t> bytes;

    bool isValid() const noexcept;
};

CompositionNeuralModelArtifact serializeCompositionNeuralModel(
    const CompositionNeuralModel& model) noexcept;

bool deserializeCompositionNeuralModel(
    const CompositionNeuralModelArtifact& artifact,
    CompositionNeuralModel& model) noexcept;

} // namespace midigengx::music
