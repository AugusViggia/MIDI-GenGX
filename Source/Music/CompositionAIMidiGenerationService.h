#pragma once

#include "CompositionNeuralGenerationService.h"
#include "MusicalEngine.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace midigengx::music
{

struct CompositionAIMidiGenerationResult
{
    std::vector<Phrase> phrases;

    std::size_t requestedPhraseCount = 0;

    bool valid = false;

    bool isValid() const noexcept;
};

CompositionAIMidiGenerationResult
generateAIMidiPhrases(
    const CompositionInferencePipeline& pipeline,
    const std::vector<double>& globalFeatures,
    const std::vector<double>& initialContextSection,
    const midigengx::domain::MusicalContext& baseContext,
    std::size_t phraseCount,
    std::uint32_t seed = 1) noexcept;

} // namespace midigengx::music
