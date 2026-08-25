#pragma once

#include "CompositionMidiSectionAnalyzer.h"
#include "MotifKnowledgeCatalog.h"
#include "MotifRecurrenceProfile.h"

namespace midigengx::music
{

struct CompositionMidiMotifAnalysis
{
    static constexpr int version = 1;

    MotifRecurrenceProfile recurrence;
    MotifKnowledgeCatalog catalog;

    bool valid = false;

    bool isValid() const noexcept;

    std::size_t totalFamilyCount() const noexcept;
    std::size_t recurringFamilyCount() const noexcept;
    double averageOccurrenceCount() const noexcept;
};

CompositionMidiMotifAnalysis analyzeCompositionMidiMotifs(
    const CompositionMidiCorpusRecord& record,
    const CompositionMidiSectionAnalysis& sections) noexcept;

} // namespace midigengx::music
