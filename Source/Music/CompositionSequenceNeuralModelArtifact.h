#pragma once

#include "CompositionSequenceNeuralModel.h"

#include <cstdint>
#include <vector>

namespace midigengx::music
{

struct CompositionSequenceNeuralModelArtifact
{
    static constexpr std::uint32_t magic = 0x4D47534Eu; // "MGSN"
    static constexpr std::uint32_t version = 1;

    std::vector<std::uint8_t> bytes;

    bool isValid() const noexcept;
};

CompositionSequenceNeuralModelArtifact
serializeCompositionSequenceNeuralModel(
    const CompositionSequenceNeuralModel& model) noexcept;

bool deserializeCompositionSequenceNeuralModel(
    const CompositionSequenceNeuralModelArtifact& artifact,
    CompositionSequenceNeuralModel& model) noexcept;

} // namespace midigengx::music
