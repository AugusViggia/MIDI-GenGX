#pragma once

#include "CompositionAIGenerationCoordinator.h"
#include "CompositionNeuralModelRuntimeLoader.h"
#include "CompositionRuntimeInferenceService.h"

#include <cstdint>

namespace midigengx::music
{

class CompositionAIModelRuntimeProvider
{
public:
    static constexpr int version = 1;

    bool isReady() const noexcept;

    bool load(
        const CompositionNeuralModelArtifact& artifact) noexcept;

    void clear() noexcept;

    Phrase generate(
        const midigengx::domain::MusicalContext& context,
        std::uint32_t seed) const noexcept;

private:
    CompositionNeuralModelRuntimeLoader loader;
    CompositionInferencePipeline pipeline;
    CompositionAIGenerationCoordinator coordinator;
};

} // namespace midigengx::music
