#pragma once

#include "CompositionComposerCorpusSourceManifest.h"
#include "CompositionComposerKnowledgeTrainingCorpus.h"

#include <cstddef>

namespace midigengx::music
{

struct CompositionComposerKnowledgeCorpusAssembly
{
    static constexpr int version = 1;

    CompositionComposerCorpusSourceManifest sourceManifest;
    CompositionComposerKnowledgeTrainingCorpus trainingCorpus;

    bool sourceIntegrityVerified = false;
    bool valid = false;

    bool isValid() const noexcept;

    std::size_t sampleCount() const noexcept;
};

CompositionComposerKnowledgeCorpusAssembly
assembleCompositionComposerKnowledgeCorpus(
    const CompositionComposerCorpusSourceManifest& sourceManifest,
    const CompositionComposerKnowledgeTrainingCorpus& trainingCorpus) noexcept;

} // namespace midigengx::music
