#pragma once

#include "CompositionComposerKnowledgeCorpusManifest.h"

namespace midigengx::music
{

struct CompositionComposerKnowledgeTrainingCorpus
{
    static constexpr int version = 1;

    CompositionComposerKnowledgeCorpusManifest manifest;

    std::vector<CompositionComposerKnowledgeSample> trainingSamples;
    std::vector<CompositionComposerKnowledgeSample> validationSamples;
    std::vector<CompositionComposerKnowledgeSample> testSamples;

    bool valid = false;

    bool isValid() const noexcept;

    std::size_t trainingCount() const noexcept;
    std::size_t validationCount() const noexcept;
    std::size_t testCount() const noexcept;
    std::size_t sampleCount() const noexcept;
};

CompositionComposerKnowledgeTrainingCorpus
buildCompositionComposerKnowledgeTrainingCorpus(
    const CompositionComposerKnowledgeCatalog& catalog,
    const CompositionComposerKnowledgePartition& partition,
    const CompositionComposerKnowledgeCorpusManifest& manifest) noexcept;

} // namespace midigengx::music
