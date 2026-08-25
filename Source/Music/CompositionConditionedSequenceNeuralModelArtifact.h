#pragma once

#include "CompositionConditionedSequenceNeuralModel.h"

#include <cstdint>
#include <vector>

namespace midigengx::music
{

struct CompositionConditionedSequenceNeuralModelArtifact
{
    static constexpr std::uint32_t magic = 0x4D47434Eu; // "MGCN"
    static constexpr std::uint32_t version = 2;

    std::vector<std::uint8_t> bytes;

    bool isValid() const noexcept;
};

CompositionConditionedSequenceNeuralModelArtifact
serializeCompositionConditionedSequenceNeuralModel(
    const CompositionConditionedSequenceNeuralModel& model) noexcept;

bool deserializeCompositionConditionedSequenceNeuralModel(
    const CompositionConditionedSequenceNeuralModelArtifact& artifact,
    CompositionConditionedSequenceNeuralModel& model) noexcept;

} // namespace midigengx::music
