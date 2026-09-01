#include "CompositionAIGenerationCoordinator.h"

namespace midigengx::music
{
namespace
{

double selectorToNormalized(int value) noexcept
{
    return (static_cast<double>(value) - 50.0) / 50.0;
}

} // namespace

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

    auto guidance =
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

    // Explicit selectors are copied into the AI guidance after model inference.
    // This preserves the priority rule: the model interprets the requested
    // musical direction, but the user's actual selector values remain explicit
    // control targets rather than being inferred from the six learned outputs.
    const auto& parameters = request.context.parameters;
    guidance.densityTarget = selectorToNormalized(parameters.density);
    guidance.catchinessTarget = selectorToNormalized(parameters.catchiness);
    guidance.syncopationTarget = selectorToNormalized(parameters.syncopation);
    guidance.octaveMovementTarget = selectorToNormalized(parameters.octaveMovement);
    guidance.variationTarget = selectorToNormalized(parameters.variation);
    guidance.repetitionTarget = selectorToNormalized(parameters.repetition);
    guidance.tensionSelectorTarget = selectorToNormalized(parameters.tension);
    guidance.complexityTarget = selectorToNormalized(parameters.complexity);
    guidance.humanizationTarget = selectorToNormalized(parameters.humanization);
    guidance.noteLengthVariationTarget = selectorToNormalized(parameters.noteLengthVariation);
    guidance.cadenceStrengthTarget = static_cast<double>(parameters.cadenceStrength) / 100.0;

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
