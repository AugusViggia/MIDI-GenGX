#pragma once

#include "CompositionMidiCorpusRecord.h"

#include <cstddef>
#include <string>
#include <vector>

namespace midigengx::music
{

struct CompositionMidiCorpusDirectoryLoadResult
{
    std::vector<CompositionMidiCorpusRecord> records;

    std::size_t discoveredFileCount = 0;
    std::size_t acceptedFileCount = 0;
    std::size_t rejectedFileCount = 0;

    bool valid = false;

    bool isValid() const noexcept;
};

CompositionMidiCorpusDirectoryLoadResult
loadCompositionMidiCorpusDirectory(
    const std::string& directoryPath,
    bool recursive) noexcept;

} // namespace midigengx::music
