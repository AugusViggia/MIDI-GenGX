#pragma once

#include "CompositionSequenceMetadata.h"

#include <cstddef>
#include <string>
#include <vector>

namespace midigengx::music
{

struct CompositionSequenceMetadataCatalog
{
    std::vector<CompositionSequenceMetadata> entries;

    bool verified = false;
    bool valid = false;

    bool isValid() const noexcept;

    const CompositionSequenceMetadata*
    findBySampleId(
        const std::string& sampleId) const noexcept;
};

CompositionSequenceMetadataCatalog
buildCompositionSequenceMetadataCatalog(
    const std::vector<CompositionSequenceMetadata>& metadata) noexcept;

} // namespace midigengx::music
