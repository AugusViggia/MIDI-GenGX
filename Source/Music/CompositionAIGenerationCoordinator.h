#pragma once

#include "MusicalEngine.h"

#include <cstdint>
#include <vector>

namespace midigengx::music
{

struct CompositionAIGenerationRequest
{
    midigengx::domain::MusicalContext context;

    std::vector<double> globalFeatures;
    std::vector<double> contextSectionFeatures;

    bool contextIsValid = false;

    std::uint32_t seed = 1;

    bool isValid(
        const CompositionLearningContract& contract) const noexcept;
};

struct CompositionAIGenerationResult
{
    Phrase phrase;
    CompositionAIGuidance guidance;

    bool usedAI = false;
    bool valid = false;

    bool isValid() const noexcept;
};

struct CompositionAIGenerationCoordinator
{
    static constexpr int version = 1;

    CompositionInferencePipeline pipeline;
    CompositionAIEngineBridge bridge;
    MusicalEngine engine;

    bool enabled = false;

    bool isValid() const noexcept;

    CompositionAIGenerationResult generate(
        const CompositionAIGenerationRequest& request) const noexcept;
};

CompositionAIGenerationCoordinator
buildCompositionAIGenerationCoordinator(
    const CompositionInferencePipeline& pipeline,
    const CompositionAIEngineBridge& bridge,
    bool enabled = true) noexcept;

} // namespace midigengx::music
