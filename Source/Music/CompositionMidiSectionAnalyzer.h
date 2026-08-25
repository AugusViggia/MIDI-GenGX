#pragma once

#include "CompositionMidiCorpusRecord.h"
#include "CompositionMidiSection.h"

#include <vector>

namespace midigengx::music
{

struct CompositionMidiSectionAnalysis
{
    static constexpr int version = 1;

    std::vector<CompositionMidiSection> sections;
    bool valid = false;

    bool isValid(
        std::uint16_t ticksPerQuarterNote) const noexcept;

    std::size_t sectionCount() const noexcept;
};

CompositionMidiSectionAnalysis
analyzeCompositionMidiSections(
    const CompositionMidiCorpusRecord& record) noexcept;

} // namespace midigengx::music
