#pragma once

#include "MotifRecurrenceProfile.h"

#include <cstddef>
#include <vector>

namespace midigengx::music
{

enum class MotifRecurrencePattern
{
    Absent,
    Single,
    Recurring,
    Periodic,
    Clustered
};

struct MotifRecurrenceMetrics
{
    MotifRecurrencePattern pattern =
        MotifRecurrencePattern::Absent;

    std::size_t occurrenceCount = 0;
    std::size_t firstPhraseIndex = 0;
    std::size_t lastPhraseIndex = 0;

    std::vector<std::size_t> phraseGaps;

    double averagePhraseGap = 0.0;
    double recurrenceDensity = 0.0;
    double transformationRate = 0.0;

    bool isValid() const noexcept;
};

MotifRecurrenceMetrics calculateMotifRecurrenceMetrics(
    const MotifRecurrenceFamily* family) noexcept;

} // namespace midigengx::music
