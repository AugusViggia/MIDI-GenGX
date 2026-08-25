#pragma once

#include "CompositionInferencePipeline.h"
#include "CompositionRuntimeFeatureAdapter.h"
#include "CompositionRuntimeInferenceAdapter.h"

namespace midigengx::music
{

struct CompositionRuntimeInferenceResult
{
    CompositionInferenceRequest request;
    CompositionInferenceResult inference;

    bool valid = false;

    bool isValid() const noexcept;
};

class CompositionRuntimeInferenceService
{
public:
    static constexpr int version = 1;

    CompositionRuntimeInferenceService() = default;

    explicit CompositionRuntimeInferenceService(
        const CompositionInferencePipeline& pipeline) noexcept;

    bool isValid() const noexcept;

    CompositionRuntimeInferenceResult infer(
        const midigengx::domain::MusicalContext& context) const noexcept;

private:
    CompositionInferencePipeline pipeline;
    CompositionRuntimeFeatureAdapter featureAdapter;
    CompositionRuntimeInferenceAdapter requestAdapter;
};

} // namespace midigengx::music
