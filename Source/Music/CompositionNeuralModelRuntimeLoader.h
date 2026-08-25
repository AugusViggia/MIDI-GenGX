#pragma once

#include "CompositionNeuralModelArtifact.h"

#include <cstddef>

namespace midigengx::music
{

struct CompositionNeuralModelRuntimeLoader
{
    static constexpr int version = 1;

    CompositionNeuralModel model;
    bool loaded = false;

    bool isValid() const noexcept;

    bool load(
        const CompositionNeuralModelArtifact& artifact) noexcept;

    void clear() noexcept;
};

} // namespace midigengx::music
