#include "CompositionRuntimeInferenceService.h"

namespace midigengx::music
{

bool CompositionRuntimeInferenceResult::isValid() const noexcept
{
    return valid &&
           request.contextIsValid &&
           inference.valid;
}

namespace
{

bool requestMatchesPipeline(
    const CompositionInferenceRequest& request,
    const CompositionInferencePipeline& pipeline) noexcept
{
    return request.isValid(
               pipeline.contract);
}

} // namespace

CompositionRuntimeInferenceService::CompositionRuntimeInferenceService(
    const CompositionInferencePipeline& inputPipeline) noexcept
    : pipeline(
          inputPipeline)
{
}

bool CompositionRuntimeInferenceService::isValid() const noexcept
{
    return pipeline.isValid();
}

CompositionRuntimeInferenceResult
CompositionRuntimeInferenceService::infer(
    const midigengx::domain::MusicalContext& context)
    const noexcept
{
    CompositionRuntimeInferenceResult result;

    if (!isValid())
        return result;

    const auto features =
        featureAdapter.build(
            context);

    if (!features.isValid())
        return result;

    result.request =
        requestAdapter.buildRequest(
            features,
            pipeline.contract);

    if (!requestMatchesPipeline(
            result.request,
            pipeline))
    {
        return result;
    }

    result.inference =
        pipeline.infer(
            result.request);

    result.valid =
        result.inference.isValid(
            pipeline.contract);

    return result;
}

} // namespace midigengx::music
