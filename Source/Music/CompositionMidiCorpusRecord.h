#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace midigengx::music
{

struct CompositionMidiNote
{
    std::uint8_t channel = 0;
    std::uint8_t midiNote = 0;
    std::uint8_t velocity = 0;

    std::uint32_t startTick = 0;
    std::uint32_t endTick = 0;

    bool isValid() const noexcept;
};

struct CompositionMidiCorpusRecord
{
    static constexpr int version = 1;

    std::string sampleId;

    std::uint16_t ticksPerQuarterNote = 0;
    std::uint16_t trackCount = 0;

    std::vector<CompositionMidiNote> notes;

    std::uint32_t lengthTicks = 0;

    bool analysisValid = false;

    bool isValid() const noexcept;

    std::size_t noteCount() const noexcept;
};

} // namespace midigengx::music
