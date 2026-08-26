#include "CompositionComposerKnowledgeRepresentation.h"

#include <algorithm>
#include <cmath>

namespace midigengx::music
{
namespace
{

constexpr double epsilon = 1.0e-9;

bool isUnit(double value) noexcept
{
    return std::isfinite(value) &&
           value >= 0.0 &&
           value <= 1.0;
}

double clampUnit(double value) noexcept
{
    return std::clamp(value, 0.0, 1.0);
}

double ratioFromEncodedCounts(
    double first,
    double second,
    double third) noexcept
{
    const double total =
        std::max(0.0, first) +
        std::max(0.0, second) +
        std::max(0.0, third);

    return total <= epsilon
        ? 0.0
        : clampUnit(first / total);
}

std::array<double, CompositionComposerKnowledgeRepresentation::featureCount>
extractFeatures(
    const CompositionComposerKnowledgeSample& sample) noexcept
{
    std::array<double, CompositionComposerKnowledgeRepresentation::featureCount> result{};

    const auto& global = sample.composition.globalFeatures;
    const auto& sections = sample.composition.sectionFeatures;

    if (global.size() < 13 || sections.empty())
        return result;

    // CompositionDatasetSample schemaVersion 1:
    // [0..5] composition/harmony/motif features,
    // [6..8] tension statistics,
    // [9..11] transition counts normalized by 64,
    // [12] peak tension.
    // Harmony quality ratios are recovered from the section-level schema,
    // because CompositionDatasetSample does not expose the six global
    // harmony counters separately. The per-section quality encoding is part
    // of the existing dataset representation.
    std::array<double, 6> harmonyCounts{};
    std::array<double, 4> phraseCounts{};

    double tensionSum = 0.0;
    double peakTension = 0.0;

    for (const auto& section : sections)
    {
        if (section.size() < 6)
            continue;

        const double role = section[0];
        const double tension = clampUnit(section[1]);
        const double quality = section[4];

        if (std::abs(role - 0.0) < 0.05)
            phraseCounts[0] += 1.0;
        else if (std::abs(role - (1.0 / 3.0)) < 0.05)
            phraseCounts[1] += 1.0;
        else if (std::abs(role - (2.0 / 3.0)) < 0.05)
            phraseCounts[2] += 1.0;
        else
            phraseCounts[3] += 1.0;

        if (std::abs(quality - 0.0) < 0.05)
            harmonyCounts[0] += 1.0;
        else if (std::abs(quality - 0.2) < 0.05)
            harmonyCounts[1] += 1.0;
        else if (std::abs(quality - 0.4) < 0.05)
            harmonyCounts[2] += 1.0;
        else if (std::abs(quality - 0.6) < 0.05)
            harmonyCounts[3] += 1.0;
        else if (std::abs(quality - 0.8) < 0.05)
            harmonyCounts[4] += 1.0;
        else
            harmonyCounts[5] += 1.0;

        tensionSum += tension;
        peakTension = std::max(peakTension, tension);
    }

    const double sectionCount =
        static_cast<double>(sections.size());

    if (sectionCount > epsilon)
    {
        for (std::size_t index = 0; index < 6; ++index)
            result[index] = clampUnit(harmonyCounts[index] / sectionCount);

        for (std::size_t index = 0; index < 4; ++index)
            result[6 + index] = clampUnit(phraseCounts[index] / sectionCount);
    }

    result[CompositionComposerKnowledgeRepresentation::AverageSectionTension] =
        clampUnit(tensionSum / std::max(sectionCount, 1.0));

    result[CompositionComposerKnowledgeRepresentation::PeakSectionTension] =
        peakTension > 0.0
            ? peakTension
            : clampUnit(global[12]);

    result[CompositionComposerKnowledgeRepresentation::RisingTransitionRatio] =
        ratioFromEncodedCounts(global[9], global[10], global[11]);

    result[CompositionComposerKnowledgeRepresentation::FallingTransitionRatio] =
        ratioFromEncodedCounts(global[10], global[9], global[11]);

    return result;
}

} // namespace

bool CompositionComposerKnowledgeRepresentation::isValid() const noexcept
{
    if (!valid ||
        composerId.empty() ||
        sampleCount == 0)
    {
        return false;
    }

    return std::all_of(
        features.begin(),
        features.end(),
        [](double value)
        {
            return isUnit(value);
        });
}

bool CompositionComposerKnowledgeSampleRepresentation::isValid() const noexcept
{
    if (!valid ||
        sampleId.empty() ||
        composerId.empty())
    {
        return false;
    }

    return std::all_of(
        features.begin(),
        features.end(),
        [](double value)
        {
            return isUnit(value);
        });
}

CompositionComposerKnowledgeSampleRepresentation
buildCompositionComposerKnowledgeSampleRepresentation(
    const CompositionComposerKnowledgeSample& sample) noexcept
{
    CompositionComposerKnowledgeSampleRepresentation result;

    if (!sample.isValid() ||
        sample.metadata.composerId.empty())
    {
        return result;
    }

    result.sampleId = sample.metadata.sampleId;
    result.composerId = sample.metadata.composerId;
    result.features = extractFeatures(sample);
    result.valid = true;

    if (!result.isValid())
        result.valid = false;

    return result;
}

CompositionComposerKnowledgeRepresentation
buildCompositionComposerKnowledgeRepresentation(
    const CompositionComposerKnowledgeGroup& group) noexcept
{
    CompositionComposerKnowledgeRepresentation result;

    if (!group.isValid())
        return result;

    std::array<double, CompositionComposerKnowledgeRepresentation::featureCount> totals{};

    for (const auto& sample : group.samples)
    {
        const auto representation =
            buildCompositionComposerKnowledgeSampleRepresentation(sample);

        if (!representation.isValid() ||
            representation.composerId != group.composerId)
        {
            return {};
        }

        for (std::size_t index = 0; index < totals.size(); ++index)
            totals[index] += representation.features[index];
    }

    result.composerId = group.composerId;
    result.sampleCount = group.samples.size();

    for (std::size_t index = 0; index < totals.size(); ++index)
    {
        result.features[index] = clampUnit(
            totals[index] /
            static_cast<double>(result.sampleCount));
    }

    result.valid = true;

    if (!result.isValid())
        result.valid = false;

    return result;
}

} // namespace midigengx::music
