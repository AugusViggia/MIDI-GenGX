#include "CompositionInferencePipeline.h"

namespace midigengx::music
{

bool CompositionInferenceRequest::isValid(
    const CompositionLearningContract& contract) const noexcept
{
    return contract.isValid() &&
           globalFeatures.size() ==
               contract.globalInputWidth &&
           contextSectionFeatures.size() ==
               contract.sectionInputWidth &&
           contextIsValid;
}

bool CompositionInferenceResult::isValid(
    const CompositionLearningContract& contract) const noexcept
{
    return valid &&
           contract.isValid() &&
           prediction.isValid(
               contract.targetWidth);
}

bool CompositionInferencePipeline::isValid() const noexcept
{
    return ready &&
           contract.isValid() &&
           model.isValid() &&
           model.contract.globalInputWidth ==
               contract.globalInputWidth &&
           model.contract.sectionInputWidth ==
               contract.sectionInputWidth &&
           model.contract.targetWidth ==
               contract.targetWidth &&
           model.contract.objective ==
               contract.objective;
}

CompositionInferenceResult
CompositionInferencePipeline::infer(
    const CompositionInferenceRequest& request) const noexcept
{
    CompositionInferenceResult result;

    if (!isValid() ||
        !request.isValid(
            contract))
    {
        return result;
    }

    result.prediction =
        model.predictNextSection(
            request.globalFeatures,
            request.contextSectionFeatures,
            request.contextIsValid);

    result.valid =
        result.prediction.isValid(
            contract.targetWidth);

    return result;
}

CompositionInferencePipeline
buildCompositionInferencePipeline(
    const CompositionNeuralModel& model) noexcept
{
    CompositionInferencePipeline pipeline;

    if (!model.isValid())
        return pipeline;

    pipeline.model =
        model;

    pipeline.contract =
        model.contract;

    pipeline.ready =
        pipeline.contract.isValid();

    return pipeline;
}

} // namespace midigengx::music
