#pragma once

#include "CompositionDatasetSample.h"
#include "CompositionSequenceMetadata.h"

namespace midigengx::music
{

struct CompositionComposerKnowledgeSample
{
    static constexpr int version = 1;

    CompositionDatasetSample composition;
    CompositionSequenceMetadata metadata;

    bool valid = false;

    bool isValid() const noexcept;
};

CompositionComposerKnowledgeSample
buildCompositionComposerKnowledgeSample(
    const CompositionDatasetSample& composition,
    const CompositionSequenceMetadata& metadata) noexcept;

} // namespace midigengx::music
