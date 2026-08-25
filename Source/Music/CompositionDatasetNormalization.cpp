#include "CompositionDatasetNormalization.h"

#include <algorithm>
#include <cmath>

namespace midigengx::music
{
namespace
{

constexpr double kEpsilon = 1.0e-12;

double safeStdDev(
    double variance) noexcept
{
    if (variance <= kEpsilon)
        return 1.0;

    return std::sqrt(variance);
}

} // namespace

bool CompositionDatasetNormalization::isValid() const noexcept
{
    if (!analysisValid ||
        globalFeatureWidth !=
            CompositionDatasetSchema::globalFeatureCount ||
        sectionFeatureWidth !=
            CompositionDatasetSchema::sectionFeatureCount ||
        globalMean.size() !=
            globalFeatureWidth ||
        globalStdDev.size() !=
            globalFeatureWidth ||
        sectionMean.size() !=
            sectionFeatureWidth ||
        sectionStdDev.size() !=
            sectionFeatureWidth)
    {
        return false;
    }

    for (const auto value :
         globalMean)
    {
        if (!std::isfinite(value))
            return false;
    }

    for (const auto value :
         globalStdDev)
    {
        if (!std::isfinite(value) ||
            value <= 0.0)
        {
            return false;
        }
    }

    for (const auto value :
         sectionMean)
    {
        if (!std::isfinite(value))
            return false;
    }

    for (const auto value :
         sectionStdDev)
    {
        if (!std::isfinite(value) ||
            value <= 0.0)
        {
            return false;
        }
    }

    return true;
}

CompositionDatasetNormalization
fitCompositionDatasetNormalization(
    const CompositionDatasetBatch& batch,
    const CompositionDatasetPartition& partition) noexcept
{
    CompositionDatasetNormalization normalization;

    if (!batch.isValid() ||
        !partition.isValid(
            batch.sampleCount))
    {
        return normalization;
    }

    normalization.globalMean.assign(
        normalization.globalFeatureWidth,
        0.0);

    normalization.globalStdDev.assign(
        normalization.globalFeatureWidth,
        1.0);

    normalization.sectionMean.assign(
        normalization.sectionFeatureWidth,
        0.0);

    normalization.sectionStdDev.assign(
        normalization.sectionFeatureWidth,
        1.0);

    if (batch.sampleCount == 0)
    {
        normalization.analysisValid = true;
        return normalization;
    }

    if (partition.trainingIndices.empty())
        return normalization;

    // Fit only from training samples. Validation/test data is never used to
    // estimate preprocessing parameters, preventing data leakage.
    for (const auto sampleIndex :
         partition.trainingIndices)
    {
        const auto globalBase =
            sampleIndex *
            batch.globalFeatureWidth;

        for (std::size_t featureIndex = 0;
             featureIndex <
                 batch.globalFeatureWidth;
             ++featureIndex)
        {
            normalization.globalMean[
                featureIndex] +=
                batch.globalMatrix[
                    globalBase +
                    featureIndex];
        }

        const auto sectionBase =
            sampleIndex *
            batch.maxSectionCount;

        for (std::size_t sectionIndex = 0;
             sectionIndex <
                 batch.maxSectionCount;
             ++sectionIndex)
        {
            if (batch.sectionMask[
                    sectionBase +
                    sectionIndex] == 0.0)
            {
                continue;
            }

            const auto featureBase =
                (sectionBase +
                 sectionIndex) *
                batch.sectionFeatureWidth;

            for (std::size_t featureIndex = 0;
                 featureIndex <
                     batch.sectionFeatureWidth;
                 ++featureIndex)
            {
                normalization.sectionMean[
                    featureIndex] +=
                    batch.sectionMatrix[
                        featureBase +
                        featureIndex];
            }
        }
    }

    const double trainingCount =
        static_cast<double>(
            partition.trainingIndices.size());

    for (auto& value :
         normalization.globalMean)
    {
        value /= trainingCount;
    }

    std::size_t trainingSectionCount = 0;

    for (const auto sampleIndex :
         partition.trainingIndices)
    {
        const auto sectionBase =
            sampleIndex *
            batch.maxSectionCount;

        for (std::size_t sectionIndex = 0;
             sectionIndex <
                 batch.maxSectionCount;
             ++sectionIndex)
        {
            if (batch.sectionMask[
                    sectionBase +
                    sectionIndex] != 0.0)
            {
                ++trainingSectionCount;
            }
        }
    }

    if (trainingSectionCount == 0)
    {
        normalization.analysisValid = true;
        return normalization;
    }

    const double sectionCount =
        static_cast<double>(
            trainingSectionCount);

    for (auto& value :
         normalization.sectionMean)
    {
        value /= sectionCount;
    }

    double globalVarianceSum = 0.0;
    std::vector<double>
        globalVariance(
            normalization.globalFeatureWidth,
            0.0);

    std::vector<double>
        sectionVariance(
            normalization.sectionFeatureWidth,
            0.0);

    for (const auto sampleIndex :
         partition.trainingIndices)
    {
        const auto globalBase =
            sampleIndex *
            batch.globalFeatureWidth;

        for (std::size_t featureIndex = 0;
             featureIndex <
                 batch.globalFeatureWidth;
             ++featureIndex)
        {
            const auto delta =
                batch.globalMatrix[
                    globalBase +
                    featureIndex] -
                normalization.globalMean[
                    featureIndex];

            globalVariance[
                featureIndex] +=
                delta * delta;
        }

        const auto sectionBase =
            sampleIndex *
            batch.maxSectionCount;

        for (std::size_t sectionIndex = 0;
             sectionIndex <
                 batch.maxSectionCount;
             ++sectionIndex)
        {
            if (batch.sectionMask[
                    sectionBase +
                    sectionIndex] == 0.0)
            {
                continue;
            }

            const auto featureBase =
                (sectionBase +
                 sectionIndex) *
                batch.sectionFeatureWidth;

            for (std::size_t featureIndex = 0;
                 featureIndex <
                     batch.sectionFeatureWidth;
                 ++featureIndex)
            {
                const auto delta =
                    batch.sectionMatrix[
                        featureBase +
                        featureIndex] -
                    normalization.sectionMean[
                        featureIndex];

                sectionVariance[
                    featureIndex] +=
                    delta * delta;
            }
        }
    }

    for (std::size_t featureIndex = 0;
         featureIndex <
             normalization.globalFeatureWidth;
         ++featureIndex)
    {
        normalization.globalStdDev[
            featureIndex] =
            safeStdDev(
                globalVariance[
                    featureIndex] /
                trainingCount);
    }

    for (std::size_t featureIndex = 0;
         featureIndex <
             normalization.sectionFeatureWidth;
         ++featureIndex)
    {
        normalization.sectionStdDev[
            featureIndex] =
            safeStdDev(
                sectionVariance[
                    featureIndex] /
                sectionCount);
    }

    normalization.analysisValid = true;
    return normalization;
}

CompositionDatasetBatch
applyCompositionDatasetNormalization(
    const CompositionDatasetBatch& batch,
    const CompositionDatasetNormalization& normalization) noexcept
{
    CompositionDatasetBatch normalized;

    if (!batch.isValid() ||
        !normalization.isValid() ||
        batch.globalFeatureWidth !=
            normalization.globalFeatureWidth ||
        batch.sectionFeatureWidth !=
            normalization.sectionFeatureWidth)
    {
        return normalized;
    }

    normalized = batch;
    normalized.analysisValid = true;

    for (std::size_t sampleIndex = 0;
         sampleIndex < batch.sampleCount;
         ++sampleIndex)
    {
        const auto globalBase =
            sampleIndex *
            batch.globalFeatureWidth;

        for (std::size_t featureIndex = 0;
             featureIndex <
                 batch.globalFeatureWidth;
             ++featureIndex)
        {
            normalized.globalMatrix[
                globalBase +
                featureIndex] =
                (batch.globalMatrix[
                     globalBase +
                     featureIndex] -
                 normalization.globalMean[
                     featureIndex]) /
                normalization.globalStdDev[
                    featureIndex];
        }

        const auto sectionBase =
            sampleIndex *
            batch.maxSectionCount;

        for (std::size_t sectionIndex = 0;
             sectionIndex <
                 batch.maxSectionCount;
             ++sectionIndex)
        {
            if (batch.sectionMask[
                    sectionBase +
                    sectionIndex] == 0.0)
            {
                continue;
            }

            const auto featureBase =
                (sectionBase +
                 sectionIndex) *
                batch.sectionFeatureWidth;

            for (std::size_t featureIndex = 0;
                 featureIndex <
                     batch.sectionFeatureWidth;
                 ++featureIndex)
            {
                normalized.sectionMatrix[
                    featureBase +
                    featureIndex] =
                    (batch.sectionMatrix[
                         featureBase +
                         featureIndex] -
                     normalization.sectionMean[
                         featureIndex]) /
                    normalization.sectionStdDev[
                         featureIndex];
            }
        }
    }

    return normalized;
}

} // namespace midigengx::music
