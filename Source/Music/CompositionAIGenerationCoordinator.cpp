#include "CompositionAIGenerationCoordinator.h"

namespace midigengx::music
{

bool CompositionAIGenerationRequest::isValid(
    const CompositionLearningContract& contract) const noexcept
{
    return contract.isValid() &&
           globalFeatures.size() ==
               contract.globalInputWidth &&
           contextSectionFeatures.size() ==
               contract.sectionInputWidth &&
           contextIsValid;
}

bool CompositionAIGenerationResult::isValid() const noexcept
{
    return valid &&
           phrase.isValid();
}

bool CompositionAIGenerationCoordinator::isValid() const noexcept
{
    return pipeline.isValid() &&
           bridge.isValid();
}

CompositionAIGenerationResult
CompositionAIGenerationCoordinator::generate(
    const CompositionAIGenerationRequest& request) const noexcept
{
    CompositionAIGenerationResult result;

    if (!enabled)
    {
        result.phrase =
            engine.generate(
                request.context,
                request.seed);

        result.usedAI = false;
        result.valid =
            result.phrase.isValid();

        return result;
    }

    if (!isValid() ||
        !request.isValid(
            pipeline.contract))
    {
        return result;
    }

    const auto inferenceRequest =
        CompositionInferenceRequest{
            request.globalFeatures,
            request.contextSectionFeatures,
            request.contextIsValid
        };

    const auto inference =
        pipeline.infer(
            inferenceRequest);

    if (!inference.isValid(
            pipeline.contract))
    {
        return result;
    }

    const auto constraintRequest =
        CompositionAIConstraintRequest{
            request.globalFeatures,
            request.contextSectionFeatures,
            request.contextIsValid
        };

    const auto constraintResult =
        adaptAIResultToMusicalConstraints(
            pipeline,
            constraintRequest);

    if (!constraintResult.isValid())
        return result;

    const auto guidance =
        bridge.deriveGuidance(
            constraintResult.profile);

    if (!guidance.isValid())
    {
        result.phrase =
            engine.generate(
                request.context,
                request.seed);

        result.usedAI = false;
        result.valid =
            result.phrase.isValid();

        return result;
    }

    result.phrase =
        engine.generateWithAIGuidance(
            request.context,
            guidance,
            request.seed);

    result.guidance =
        guidance;

    result.usedAI = true;
    result.valid =
        result.phrase.isValid();

    return result;
}

CompositionAIGenerationCoordinator
buildCompositionAIGenerationCoordinator(
    const CompositionInferencePipeline& pipeline,
    const CompositionAIEngineBridge& bridge,
    bool enabled) noexcept
{
    CompositionAIGenerationCoordinator coordinator;

    coordinator.pipeline =
        pipeline;

    coordinator.bridge =
        bridge;

    coordinator.enabled =
        enabled;

    return coordinator;
}

} // namespace midigengx::music
