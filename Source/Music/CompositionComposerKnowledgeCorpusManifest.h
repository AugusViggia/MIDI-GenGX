#pragma once

#include "CompositionComposerKnowledgeCatalog.h"
#include "CompositionComposerKnowledgePartition.h"

#include <cstdint>
#include <string>
#include <vector>

namespace midigengx::music
{

struct CompositionComposerKnowledgeCorpusManifest
{
    static constexpr std::uint32_t magic = 0x4D47434Du; // "MGCM"
    static constexpr std::uint32_t version = 1;

    std::string corpusId;
    std::string corpusVersion;

    std::size_t composerCount = 0;
    std::size_t sampleCount = 0;
    std::size_t workCount = 0;

    double validationRatio = 0.0;
    double testRatio = 0.0;

    std::size_t trainingCount = 0;
    std::size_t validationCount = 0;
    std::size_t testCount = 0;

    std::vector<std::string> composerIds;

    bool verified = false;
    bool valid = false;

    bool isValid() const noexcept;
};

CompositionComposerKnowledgeCorpusManifest
buildCompositionComposerKnowledgeCorpusManifest(
    const CompositionComposerKnowledgeCatalog& catalog,
    const CompositionComposerKnowledgePartition& partition,
    const std::string& corpusId,
    const std::string& corpusVersion) noexcept;

} // namespace midigengx::music
