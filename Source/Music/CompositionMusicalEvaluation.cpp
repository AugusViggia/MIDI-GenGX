#include "CompositionMusicalEvaluation.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace midigengx::music
{
namespace
{

constexpr std::size_t kRoleIndex = 0;
constexpr std::size_t kTensionIndex = 1;
constexpr std::size_t kTensionDeltaIndex = 2;
constexpr std::size_t kHarmonyDegreeIndex = 3;
constexpr std::size_t kHarmonyQualityIndex = 4;
constexpr std::size_t kHarmonicDegreeDeltaIndex = 5;

const std::vector<std::size_t>& selectedIndices(
    const CompositionDatasetPreparedView& prepared,
    bool useValidationSet) noexcept
{
    return useValidationSet
               ? prepared.partition.validationIndices
               : prepared.partition.testIndices;
}

double consistencyScore(
    double predictedDelta,
    double impliedDelta) noexcept
{
    return std::clamp(
        1.0 -
            std::abs(
                predictedDelta -
                impliedDelta),
        0.0,
        1.0);
}

double rangeScore(
    const std::vector<double>& values) noexcept
{
    if (values.empty())
        return 0.0;

    std::size_t validCount = 0;

    for (const auto value :
         values)
    {
        if (std::isfinite(value) &&
            value >= -1.0 &&
            value <= 1.0)
        {
            ++validCount;
        }
    }

    return static_cast<double>(
               validCount) /
           static_cast<double>(
               values.size());
}

CompositionMusicalEvaluationResult
evaluateIndices(
    const CompositionNeuralModel& model,
    const CompositionDatasetPreparedView& prepared,
    const std::vector<std::size_t>& indices) noexcept
{
    CompositionMusicalEvaluationResult result;

    if (!model.isValid() ||
        !prepared.isValid() ||
        indices.empty())
    {
        return result;
    }

    const auto& batch =
        prepared.normalizedBatch;

    double structuralScore = 0.0;
    double tensionScore = 0.0;
    double harmonicScore = 0.0;
    double rangeScoreTotal = 0.0;

    std::size_t examples = 0;

    for (const auto sampleIndex :
         indices)
    {
        if (sampleIndex >=
            batch.sampleCount)
        {
            return CompositionMusicalEvaluationResult{};
        }

        const auto sampleBase =
            sampleIndex *
            batch.maxSectionCount;

        const auto globalBase =
            sampleIndex *
            batch.globalFeatureWidth;

        std::vector<double> globalFeatures(
            batch.globalMatrix.begin() +
                globalBase,
            batch.globalMatrix.begin() +
                globalBase +
                batch.globalFeatureWidth);

        for (std::size_t sectionIndex = 0;
             sectionIndex + 1 <
                 batch.maxSectionCount;
             ++sectionIndex)
        {
            if (batch.sectionMask[
                    sampleBase +
                    sectionIndex] == 0.0 ||
                batch.sectionMask[
                    sampleBase +
                    sectionIndex +
                    1] == 0.0)
            {
                continue;
            }

            const auto contextBase =
                (sampleBase +
                 sectionIndex) *
                batch.sectionFeatureWidth;

            std::vector<double> contextFeatures(
                batch.sectionMatrix.begin() +
                    contextBase,
                batch.sectionMatrix.begin() +
                    contextBase +
                    batch.sectionFeatureWidth);

            const auto prediction =
                model.predictNextSection(
                    globalFeatures,
                    contextFeatures,
                    true);

            if (!prediction.isValid(
                    batch.sectionFeatureWidth))
            {
                return CompositionMusicalEvaluationResult{};
            }

            const auto& p =
                prediction.sectionFeatures;

            const auto impliedTensionDelta =
                p[kTensionIndex] -
                contextFeatures[
                    kTensionIndex];

            const auto impliedHarmonicDelta =
                p[kHarmonyDegreeIndex] -
                contextFeatures[
                    kHarmonyDegreeIndex];

            const auto tensionConsistency =
                consistencyScore(
                    p[kTensionDeltaIndex],
                    impliedTensionDelta);

            const auto harmonicConsistency =
                consistencyScore(
                    p[kHarmonicDegreeDeltaIndex],
                    impliedHarmonicDelta);

            const auto rangeValidity =
                rangeScore(
                    p);

            // The role encoding is continuous in the current schema.
            // Penalize extreme jumps while allowing genuine development.
            const auto roleDistance =
                std::abs(
                    p[kRoleIndex] -
                    contextFeatures[
                        kRoleIndex]);

            const auto roleContinuity =
                std::clamp(
                    1.0 -
                        roleDistance,
                    0.0,
                    1.0);

            structuralScore +=
                0.5 *
                    roleContinuity +
                0.5 *
                    rangeValidity;

            tensionScore +=
                tensionConsistency;

            harmonicScore +=
                harmonicConsistency;

            rangeScoreTotal +=
                rangeValidity;

            ++examples;
        }
    }

    if (examples == 0)
        return result;

    result.exampleCount =
        examples;

    result.structuralCoherenceScore =
        structuralScore /
        static_cast<double>(
            examples);

    result.tensionConsistencyScore =
        tensionScore /
        static_cast<double>(
            examples);

    result.harmonicConsistencyScore =
        harmonicScore /
        static_cast<double>(
            examples);

    result.rangeValidityScore =
        rangeScoreTotal /
        static_cast<double>(
            examples);

    result.overallScore =
        0.30 *
            result.structuralCoherenceScore +
        0.30 *
            result.tensionConsistencyScore +
        0.30 *
            result.harmonicConsistencyScore +
        0.10 *
            result.rangeValidityScore;

    result.valid =
        std::isfinite(
            result.overallScore) &&
        result.overallScore >= 0.0 &&
        result.overallScore <= 1.0;

    return result;
}

} // namespace

bool CompositionMusicalEvaluationResult::isValid() const noexcept
{
    return valid &&
           exampleCount > 0 &&
           structuralCoherenceScore >= 0.0 &&
           structuralCoherenceScore <= 1.0 &&
           tensionConsistencyScore >= 0.0 &&
           tensionConsistencyScore <= 1.0 &&
           harmonicConsistencyScore >= 0.0 &&
           harmonicConsistencyScore <= 1.0 &&
           rangeValidityScore >= 0.0 &&
           rangeValidityScore <= 1.0 &&
           overallScore >= 0.0 &&
           overallScore <= 1.0;
}

CompositionMusicalEvaluationResult
evaluateCompositionNeuralMusicalQuality(
    const CompositionNeuralModel& model,
    const CompositionDatasetPreparedView& prepared,
    bool useValidationSet) noexcept
{
    return evaluateIndices(
        model,
        prepared,
        selectedIndices(
            prepared,
            useValidationSet));
}

} // namespace midigengx::music
