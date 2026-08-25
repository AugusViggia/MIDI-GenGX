#pragma once

#include "CompositionInferencePipeline.h"
#include "CompositionRuntimeFeatureAdapter.h"

namespace midigengx::music
{

struct CompositionRuntimeInferenceAdapter
{
    static constexpr int version = 1;

    CompositionInferenceRequest buildRequest(
        const CompositionRuntimeFeatures& features,
        const CompositionLearningContract& contract) const noexcept;
};

} // namespace midigengx::music
