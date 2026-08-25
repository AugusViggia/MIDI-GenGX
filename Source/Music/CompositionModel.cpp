#include "CompositionModel.h"

#include <cmath>

namespace midigengx::music
{

bool CompositionModelPrediction::isValid(
    std::size_t expectedWidth) const noexcept
{
    if (!valid ||
        sectionFeatures.size() !=
            expectedWidth)
    {
        return false;
    }

    for (const auto value :
         sectionFeatures)
    {
        if (!std::isfinite(value))
            return false;
    }

    return true;
}

bool CompositionModel::isValid() const noexcept
{
    return trained &&
           contract.isValid() &&
           contract.objective ==
               LearningObjective::NextSectionPrediction &&
           nextSectionMean.size() ==
               contract.targetWidth;
}

CompositionModelPrediction
CompositionModel::predictNextSection(
    const std::vector<double>& globalFeatures,
    const std::vector<double>& contextSectionFeatures,
    bool contextIsValid) const noexcept
{
    CompositionModelPrediction prediction;

    if (!isValid() ||
        !contextIsValid ||
        globalFeatures.size() !=
            contract.globalInputWidth ||
        contextSectionFeatures.size() !=
            contract.sectionInputWidth)
    {
        return prediction;
    }

    prediction.sectionFeatures =
        nextSectionMean;

    prediction.valid = true;
    return prediction;
}

CompositionModel
trainCompositionBaselineModel(
    const CompositionDatasetPreparedView& prepared,
    const CompositionLearningContract& contract) noexcept
{
    CompositionModel model;

    if (!prepared.isValid() ||
        !contract.isValid() ||
        contract.objective !=
            LearningObjective::NextSectionPrediction ||
        contract.globalInputWidth !=
            prepared.normalizedBatch.globalFeatureWidth ||
        contract.sectionInputWidth !=
            prepared.normalizedBatch.sectionFeatureWidth ||
        prepared.trainingCount() == 0)
    {
        return model;
    }

    model.contract =
        contract;

    model.nextSectionMean.assign(
        contract.targetWidth,
        0.0);

    std::size_t trainingExampleCount = 0;

    const auto& batch =
        prepared.normalizedBatch;

    for (const auto sampleIndex :
         prepared.partition.trainingIndices)
    {
        if (sampleIndex >=
            batch.sampleCount)
        {
            return CompositionModel{};
        }

        const auto sectionBase =
            sampleIndex *
            batch.maxSectionCount;

        bool hasContext = false;

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

            hasContext = true;

            if (sectionIndex + 1 >=
                batch.maxSectionCount ||
                batch.sectionMask[
                    sectionBase +
                    sectionIndex +
                    1] == 0.0)
            {
                break;
            }

            const auto targetBase =
                (sectionBase +
                 sectionIndex +
                 1) *
                batch.sectionFeatureWidth;

            for (std::size_t featureIndex = 0;
                 featureIndex <
                     contract.targetWidth;
                 ++featureIndex)
            {
                model.nextSectionMean[
                    featureIndex] +=
                    batch.sectionMatrix[
                        targetBase +
                        featureIndex];
            }

            ++trainingExampleCount;
        }

        if (!hasContext)
            return CompositionModel{};
    }

    if (trainingExampleCount == 0)
        return CompositionModel{};

    const double count =
        static_cast<double>(
            trainingExampleCount);

    for (auto& value :
         model.nextSectionMean)
    {
        value /= count;
    }

    model.trained = true;
    return model;
}

} // namespace midigengx::music
