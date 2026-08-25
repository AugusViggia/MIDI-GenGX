#include "CompositionRuntimeInferenceAdapter.h"

namespace midigengx::music
{

CompositionInferenceRequest
CompositionRuntimeInferenceAdapter::buildRequest(
    const CompositionRuntimeFeatures& features,
    const CompositionLearningContract& contract)
    const noexcept
{
    CompositionInferenceRequest request;

    if (!contract.isValid() ||
        !features.isValid() ||
        features.globalFeatures.size() !=
            contract.globalInputWidth ||
        features.sectionFeatures.size() !=
            contract.sectionInputWidth)
    {
        return request;
    }

    request.globalFeatures =
        features.globalFeatures;

    request.contextSectionFeatures =
        features.sectionFeatures;

    request.contextIsValid =
        true;

    return request;
}

} // namespace midigengx::music
