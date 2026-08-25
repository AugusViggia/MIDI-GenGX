#include "CompositionModelEvaluation.h"

#include <cmath>
#include <vector>

namespace midigengx::music
{
namespace
{

const std::vector<std::size_t>& selectedIndices(
    const CompositionDatasetPreparedView& prepared,
    bool useValidationSet) noexcept
{
    return useValidationSet
               ? prepared.partition.validationIndices
               : prepared.partition.testIndices;
}

void accumulateErrors(
    const std::vector<double>& prediction,
    const std::vector<double>& target,
    double& squared,
    double& absolute) noexcept
{
    for (std::size_t index = 0;
         index < prediction.size();
         ++index)
    {
        const auto error =
            prediction[index] -
            target[index];

        squared +=
            error * error;

        absolute +=
            std::abs(error);
    }
}

template <typename Predictor>
CompositionModelEvaluationResult evaluateIndices(
    const CompositionDatasetPreparedView& prepared,
    const std::vector<std::size_t>& indices,
    Predictor predictor) noexcept
{
    CompositionModelEvaluationResult result;

    if (!prepared.isValid() ||
        indices.empty())
    {
        return result;
    }

    const auto& batch =
        prepared.normalizedBatch;

    double squared = 0.0;
    double absolute = 0.0;
    std::size_t exampleCount = 0;

    for (const auto sampleIndex :
         indices)
    {
        if (sampleIndex >=
            batch.sampleCount)
        {
            return CompositionModelEvaluationResult{};
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

            const auto targetBase =
                (sampleBase +
                 sectionIndex +
                 1) *
                batch.sectionFeatureWidth;

            std::vector<double> contextFeatures(
                batch.sectionMatrix.begin() +
                    contextBase,
                batch.sectionMatrix.begin() +
                    contextBase +
                    batch.sectionFeatureWidth);

            const auto prediction =
                predictor(
                    globalFeatures,
                    contextFeatures);

            if (!prediction.isValid(
                    batch.sectionFeatureWidth))
            {
                return CompositionModelEvaluationResult{};
            }

            std::vector<double> target(
                batch.sectionMatrix.begin() +
                    targetBase,
                batch.sectionMatrix.begin() +
                    targetBase +
                    batch.sectionFeatureWidth);

            accumulateErrors(
                prediction.sectionFeatures,
                target,
                squared,
                absolute);

            ++exampleCount;
        }
    }

    if (exampleCount == 0)
        return result;

    const auto denominator =
        static_cast<double>(
            exampleCount *
            batch.sectionFeatureWidth);

    result.sampleCount =
        indices.size();

    result.exampleCount =
        exampleCount;

    result.meanSquaredError =
        squared /
        denominator;

    result.meanAbsoluteError =
        absolute /
        denominator;

    result.valid =
        std::isfinite(
            result.meanSquaredError) &&
        std::isfinite(
            result.meanAbsoluteError);

    return result;
}

} // namespace

bool CompositionModelEvaluationResult::isValid() const noexcept
{
    return valid &&
           sampleCount > 0 &&
           exampleCount > 0 &&
           std::isfinite(
               meanSquaredError) &&
           std::isfinite(
               meanAbsoluteError) &&
           meanSquaredError >= 0.0 &&
           meanAbsoluteError >= 0.0;
}

CompositionModelEvaluationResult
evaluateCompositionBaselineModel(
    const CompositionModel& model,
    const CompositionDatasetPreparedView& prepared,
    bool useValidationSet) noexcept
{
    if (!model.isValid())
        return {};

    return evaluateIndices(
        prepared,
        selectedIndices(
            prepared,
            useValidationSet),
        [&model](
            const std::vector<double>& global,
            const std::vector<double>& context)
        {
            return model.predictNextSection(
                global,
                context,
                true);
        });
}

CompositionModelEvaluationResult
evaluateCompositionNeuralModel(
    const CompositionNeuralModel& model,
    const CompositionDatasetPreparedView& prepared,
    bool useValidationSet) noexcept
{
    if (!model.isValid())
        return {};

    return evaluateIndices(
        prepared,
        selectedIndices(
            prepared,
            useValidationSet),
        [&model](
            const std::vector<double>& global,
            const std::vector<double>& context)
        {
            return model.predictNextSection(
                global,
                context,
                true);
        });
}

} // namespace midigengx::music
