#pragma once

#include "CompositionComposerKnowledgeCatalog.h"

#include <cstddef>
#include <string>
#include <vector>

namespace midigengx::music
{

enum class ComposerKnowledgeSplit
{
    Training,
    Validation,
    Test
};

struct CompositionComposerKnowledgePartition
{
    static constexpr int version = 1;

    std::vector<std::size_t> trainingSampleIndices;
    std::vector<std::size_t> validationSampleIndices;
    std::vector<std::size_t> testSampleIndices;

    double validationRatio = 0.0;
    double testRatio = 0.0;

    bool analysisValid = false;

    bool isValid(
        std::size_t sampleCount) const noexcept;

    std::size_t trainingCount() const noexcept;
    std::size_t validationCount() const noexcept;
    std::size_t testCount() const noexcept;

    bool contains(
        ComposerKnowledgeSplit split,
        std::size_t sampleIndex) const noexcept;
};

CompositionComposerKnowledgePartition
buildCompositionComposerKnowledgePartition(
    const CompositionComposerKnowledgeCatalog& catalog,
    double validationRatio,
    double testRatio) noexcept;

} // namespace midigengx::music
