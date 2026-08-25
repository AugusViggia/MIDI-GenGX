#include "MotifKnowledgeRecord.h"

#include <cmath>

namespace midigengx::music
{

bool MotifKnowledgeRecord::isRecurring() const noexcept
{
    return occurrenceCount > 1;
}

bool MotifKnowledgeRecord::isValid() const noexcept
{
    if (canonicalKey.empty() ||
        noteCount == 0 ||
        !std::isfinite(lengthBeats) ||
        lengthBeats <= 0.0 ||
        !std::isfinite(averagePhraseGap) ||
        !std::isfinite(recurrenceDensity) ||
        !std::isfinite(transformationRate) ||
        recurrenceDensity < 0.0 ||
        recurrenceDensity > 1.0 ||
        transformationRate < 0.0 ||
        transformationRate > 1.0 ||
        occurrenceCount == 0)
    {
        return false;
    }

    if (firstPhraseIndex > lastPhraseIndex)
        return false;

    if (occurrenceCount == 1 &&
        recurrencePattern !=
            MotifRecurrencePattern::Single)
    {
        return false;
    }

    if (occurrenceCount > 1 &&
        !isRecurring())
    {
        return false;
    }

    return true;
}

MotifKnowledgeRecord buildMotifKnowledgeRecord(
    const MotifRecurrenceFamily& family,
    const MotifRecurrenceMetrics& metrics) noexcept
{
    MotifKnowledgeRecord record;

    if (!family.fingerprint.isValid() ||
        !metrics.isValid())
    {
        return record;
    }

    record.canonicalKey =
        family.fingerprint.canonicalKey();

    record.noteCount =
        family.fingerprint.noteCount;

    record.lengthBeats =
        family.fingerprint.lengthBeats;

    record.firstPitchAnchor =
        family.fingerprint.firstPitchAnchor;

    record.recurrencePattern =
        metrics.pattern;

    record.occurrenceCount =
        metrics.occurrenceCount;

    record.firstPhraseIndex =
        metrics.firstPhraseIndex;

    record.lastPhraseIndex =
        metrics.lastPhraseIndex;

    record.averagePhraseGap =
        metrics.averagePhraseGap;

    record.recurrenceDensity =
        metrics.recurrenceDensity;

    record.transformationRate =
        metrics.transformationRate;

    record.transpositionCount =
        family.transpositionCount;

    record.retrogradeCount =
        family.retrogradeCount;

    record.inversionCount =
        family.inversionCount;

    record.rhythmicVariationCount =
        family.rhythmicVariationCount;

    record.intervalVariationCount =
        family.intervalVariationCount;

    record.compoundVariationCount =
        family.compoundVariationCount;

    return record;
}

} // namespace midigengx::music
