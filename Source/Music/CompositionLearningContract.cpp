#include "CompositionLearningContract.h"

namespace midigengx::music
{

bool CompositionLearningContract::isValid() const noexcept
{
    if (!analysisValid ||
        globalInputWidth !=
            CompositionDatasetSchema::globalFeatureCount ||
        sectionInputWidth !=
            CompositionDatasetSchema::sectionFeatureCount ||
        targetWidth !=
            CompositionDatasetSchema::sectionFeatureCount ||
        !usesSectionMask)
    {
        return false;
    }

    if (objective ==
            LearningObjective::NextSectionPrediction &&
        contextLength == 0)
    {
        return false;
    }

    return true;
}

CompositionLearningContract
buildCompositionLearningContract(
    const CompositionDatasetPreparedView& prepared,
    LearningObjective objective) noexcept
{
    CompositionLearningContract contract;

    if (!prepared.isValid())
        return contract;

    contract.objective =
        objective;

    contract.analysisValid = true;

    contract.globalInputWidth =
        prepared.normalizedBatch.globalFeatureWidth;

    contract.sectionInputWidth =
        prepared.normalizedBatch.sectionFeatureWidth;

    contract.targetWidth =
        prepared.normalizedBatch.sectionFeatureWidth;

    contract.contextLength =
        prepared.normalizedBatch.maxSectionCount;

    contract.usesSectionMask =
        !prepared.normalizedBatch
             .sectionMask.empty();

    // Next-section prediction requires at least one section of context.
    if (objective ==
            LearningObjective::NextSectionPrediction &&
        contract.contextLength == 0)
    {
        contract.analysisValid = false;
    }

    return contract;
}

} // namespace midigengx::music
