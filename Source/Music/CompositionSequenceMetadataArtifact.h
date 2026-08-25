#pragma once

#include "CompositionSequenceMetadataCatalog.h"

#include <cstdint>
#include <vector>

namespace midigengx::music
{

struct CompositionSequenceMetadataArtifact
{
    static constexpr std::uint32_t magic = 0x4D47544Du; // "MGTM"
    static constexpr std::uint32_t version = 1;

    std::vector<std::uint8_t> bytes;

    bool isValid() const noexcept;
};

CompositionSequenceMetadataArtifact
serializeCompositionSequenceMetadataCatalog(
    const CompositionSequenceMetadataCatalog& catalog) noexcept;

bool deserializeCompositionSequenceMetadataCatalog(
    const CompositionSequenceMetadataArtifact& artifact,
    CompositionSequenceMetadataCatalog& catalog) noexcept;

} // namespace midigengx::music
