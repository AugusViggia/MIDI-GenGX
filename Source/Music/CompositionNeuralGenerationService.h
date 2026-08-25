#pragma once

#include "CompositionInferencePipeline.h"

#include <cstddef>
#include <vector>

namespace midigengx::music
{

struct CompositionNeuralGenerationResult
{
    std::vector<std::vector<double>> generatedSections;

    std::size_t requestedSectionCount = 0;

    bool valid = false;

    bool isValid(
        std::size_t expectedWidth) const noexcept;
};

CompositionNeuralGenerationResult
generateNextSections(
    const CompositionInferencePipeline& pipeline,
    const std::vector<double>& globalFeatures,
    const std::vector<double>& initialContextSection,
    std::size_t sectionCount) noexcept;

} // namespace midigengx::music
