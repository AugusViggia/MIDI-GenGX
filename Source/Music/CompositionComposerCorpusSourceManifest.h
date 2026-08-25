#pragma once

#include "CompositionComposerKnowledgeCatalog.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace midigengx::music
{

struct CompositionComposerCorpusSourceEntry
{
    std::string sampleId;
    std::string relativeMidiPath;

    std::uint64_t byteSize = 0;
    std::uint64_t contentHash = 0;

    bool valid = false;

    bool isValid() const noexcept;
};

struct CompositionComposerCorpusSourceManifest
{
    static constexpr int version = 1;

    std::string corpusRoot;
    std::vector<CompositionComposerCorpusSourceEntry> entries;

    bool valid = false;

    bool isValid() const noexcept;

    std::size_t sampleCount() const noexcept;
};

CompositionComposerCorpusSourceManifest
buildCompositionComposerCorpusSourceManifest(
    const std::string& midiDirectoryPath,
    const CompositionComposerKnowledgeCatalog& catalog,
    bool recursive) noexcept;

} // namespace midigengx::music
