#pragma once

#include "CompositionMidiCorpusRecord.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace midigengx::music
{

struct CompositionMidiFileCorpusReader
{
    static constexpr int version = 1;

    CompositionMidiCorpusRecord read(
        const std::string& sampleId,
        const std::uint8_t* data,
        std::size_t size) const noexcept;
};

} // namespace midigengx::music
