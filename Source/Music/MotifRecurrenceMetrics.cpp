#include "MotifRecurrenceMetrics.h"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace midigengx::music
{
namespace
{

constexpr double kEpsilon = 1.0e-9;

MotifRecurrencePattern classifyPattern(
    const std::vector<std::size_t>& gaps) noexcept
{
    if (gaps.empty())
        return MotifRecurrencePattern::Absent;

    if (gaps.size() == 1)
        return MotifRecurrencePattern::Recurring;

    const auto minimum =
        *std::min_element(
            gaps.begin(),
            gaps.end());

    const auto maximum =
        *std::max_element(
            gaps.begin(),
            gaps.end());

    if (minimum == maximum)
        return MotifRecurrencePattern::Periodic;

    const double average =
        std::accumulate(
            gaps.begin(),
            gaps.end(),
            0.0) /
        static_cast<double>(
            gaps.size());

    const std::size_t clusteredThreshold =
        std::max<std::size_t>(
            1,
            static_cast<std::size_t>(
                std::ceil(
                    average * 0.5)));

    std::size_t clusteredGaps = 0;

    for (const auto gap :
         gaps)
    {
        if (gap <= clusteredThreshold)
            ++clusteredGaps;
    }

    if (clusteredGaps * 2 >= gaps.size())
        return MotifRecurrencePattern::Clustered;

    return MotifRecurrencePattern::Recurring;
}

} // namespace

bool MotifRecurrenceMetrics::isValid() const noexcept
{
    if (pattern == MotifRecurrencePattern::Absent)
        return occurrenceCount == 0;

    if (occurrenceCount == 0 ||
        firstPhraseIndex > lastPhraseIndex ||
        !std::isfinite(
            averagePhraseGap) ||
        !std::isfinite(
            recurrenceDensity) ||
        !std::isfinite(
            transformationRate) ||
        recurrenceDensity < -kEpsilon ||
        recurrenceDensity > 1.0 + kEpsilon ||
        transformationRate < -kEpsilon)
    {
        return false;
    }

    if (occurrenceCount == 1)
        return phraseGaps.empty();

    return phraseGaps.size() ==
           occurrenceCount - 1;
}

MotifRecurrenceMetrics
calculateMotifRecurrenceMetrics(
    const MotifRecurrenceFamily* family) noexcept
{
    MotifRecurrenceMetrics metrics;

    if (family == nullptr ||
        family->occurrenceIndices.empty())
    {
        return metrics;
    }

    metrics.occurrenceCount =
        family->occurrenceIndices.size();

    metrics.firstPhraseIndex =
        family->firstPhraseIndex;

    metrics.lastPhraseIndex =
        family->lastPhraseIndex;

    if (metrics.occurrenceCount == 1)
    {
        metrics.pattern =
            MotifRecurrencePattern::Single;
        return metrics;
    }

    // The recurrence family stores occurrence indices, but the recurrence
    // profile guarantees they are ordered by source-phrase traversal.
    std::vector<std::size_t> phrasePositions;
    phrasePositions.reserve(
        metrics.occurrenceCount);

    // The family API intentionally stores only the phrase span. For this
    // phase, occurrence index spacing is used as the deterministic recurrence
    // proxy. This keeps the metrics layer independent of the source graph.
    phrasePositions =
        family->phraseIndices;

    std::sort(
        phrasePositions.begin(),
        phrasePositions.end());

    for (std::size_t i = 1;
         i < phrasePositions.size();
         ++i)
    {
        metrics.phraseGaps.push_back(
            phrasePositions[i] -
            phrasePositions[i - 1]);
    }

    if (!metrics.phraseGaps.empty())
    {
        const auto totalGap =
            std::accumulate(
                metrics.phraseGaps.begin(),
                metrics.phraseGaps.end(),
                std::size_t { 0 });

        metrics.averagePhraseGap =
            static_cast<double>(
                totalGap) /
            static_cast<double>(
                metrics.phraseGaps.size());

        const auto span =
            metrics.lastPhraseIndex -
            metrics.firstPhraseIndex;

        if (span > 0)
        {
            metrics.recurrenceDensity =
                static_cast<double>(
                    metrics.occurrenceCount - 1) /
                static_cast<double>(
                    span);
        }
    }

    metrics.transformationRate =
        metrics.occurrenceCount > 1
            ? std::min(
                  1.0,
                  static_cast<double>(
                      family->transformationCount()) /
                  static_cast<double>(
                      metrics.occurrenceCount *
                      (metrics.occurrenceCount - 1) /
                      2))
            : 0.0;

    metrics.pattern =
        classifyPattern(
            metrics.phraseGaps);

    return metrics;
}

} // namespace midigengx::music
