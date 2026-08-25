#pragma once

#include "CompositionConditionedSequenceNeuralModelArtifact.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace midigengx::music
{

struct CompositionConditionedSequenceNeuralModelRuntimeLoadResult
{
    CompositionConditionedSequenceNeuralModel model;
    CompositionConditionedSequenceNeuralModelArtifact artifact;

    bool valid = false;

    bool isValid() const noexcept;
};

class CompositionConditionedSequenceNeuralModelRuntimeLoader
{
public:
    static constexpr int version = 1;

    CompositionConditionedSequenceNeuralModelRuntimeLoadResult
    load(
        const std::vector<std::uint8_t>& embeddedBytes) const noexcept;

    CompositionConditionedSequenceNeuralModelRuntimeLoadResult
    load(
        const std::uint8_t* data,
        std::size_t size) const noexcept;
};

} // namespace midigengx::music
