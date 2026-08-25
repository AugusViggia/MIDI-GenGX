#pragma once

#include "CompositionSequenceMetadataCatalog.h"

#include <string>

namespace midigengx::music
{

struct CompositionSequenceMetadataFileLoadResult
{
    CompositionSequenceMetadataCatalog catalog;

    std::size_t parsedEntryCount = 0;
    std::size_t rejectedLineCount = 0;

    bool valid = false;

    bool isValid() const noexcept;
};

CompositionSequenceMetadataFileLoadResult
loadCompositionSequenceMetadataFile(
    const std::string& filePath) noexcept;

} // namespace midigengx::music
