#pragma once

#include "MotifRecurrenceMetrics.h"

#include <cstddef>
#include <string>

namespace midigengx::music
{

struct MotifKnowledgeRecord
{
    std::string canonicalKey;

    std::size_t noteCount = 0;
    double lengthBeats = 0.0;

    int firstPitchAnchor = 0;

    MotifRecurrencePattern recurrencePattern =
        MotifRecurrencePattern::Absent;

    std::size_t occurrenceCount = 0;
    std::size_t firstPhraseIndex = 0;
    std::size_t lastPhraseIndex = 0;

    double averagePhraseGap = 0.0;
    double recurrenceDensity = 0.0;
    double transformationRate = 0.0;

    std::size_t transpositionCount = 0;
    std::size_t retrogradeCount = 0;
    std::size_t inversionCount = 0;
    std::size_t rhythmicVariationCount = 0;
    std::size_t intervalVariationCount = 0;
    std::size_t compoundVariationCount = 0;

    bool isRecurring() const noexcept;
    bool isValid() const noexcept;
};

MotifKnowledgeRecord buildMotifKnowledgeRecord(
    const MotifRecurrenceFamily& family,
    const MotifRecurrenceMetrics& metrics) noexcept;

} // namespace midigengx::music
