#pragma once

#include "CompositionMusicalEvaluation.h"

#include <cstddef>
#include <vector>

namespace midigengx::music
{

struct CompositionInferenceRequest
{
    std::vector<double> globalFeatures;
    std::vector<double> contextSectionFeatures;
    bool contextIsValid = false;

    bool isValid(
        const CompositionLearningContract& contract) const noexcept;
};

struct CompositionInferenceResult
{
    CompositionNeuralPrediction prediction;

    bool valid = false;

    bool isValid(
        const CompositionLearningContract& contract) const noexcept;
};

struct CompositionInferencePipeline
{
    static constexpr int version = 1;

    CompositionNeuralModel model;
    CompositionLearningContract contract;

    bool ready = false;

    bool isValid() const noexcept;

    CompositionInferenceResult infer(
        const CompositionInferenceRequest& request) const noexcept;
};

CompositionInferencePipeline buildCompositionInferencePipeline(
    const CompositionNeuralModel& model) noexcept;

} // namespace midigengx::music
