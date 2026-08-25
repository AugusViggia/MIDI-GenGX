#pragma once

#include "CompositionComposerKnowledgeSample.h"

#include <cstddef>
#include <string>
#include <vector>

namespace midigengx::music
{

struct CompositionComposerKnowledgeGroup
{
    std::string composerId;
    std::vector<CompositionComposerKnowledgeSample> samples;

    bool isValid() const noexcept;
    std::size_t sampleCount() const noexcept;
};

struct CompositionComposerKnowledgeCatalog
{
    static constexpr int version = 1;

    std::vector<CompositionComposerKnowledgeGroup> composers;
    bool verified = false;
    bool valid = false;

    bool isValid() const noexcept;
    std::size_t composerCount() const noexcept;
    std::size_t sampleCount() const noexcept;

    const CompositionComposerKnowledgeGroup*
    findComposer(
        const std::string& composerId) const noexcept;
};

CompositionComposerKnowledgeCatalog
buildCompositionComposerKnowledgeCatalog(
    const std::vector<CompositionComposerKnowledgeSample>& samples,
    bool requireVerifiedMetadata = true) noexcept;

} // namespace midigengx::music
