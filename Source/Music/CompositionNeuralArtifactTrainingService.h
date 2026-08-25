#pragma once

#include "CompositionNeuralModelArtifact.h"
#include "CompositionNeuralTrainer.h"

namespace midigengx::music
{

struct CompositionNeuralArtifactTrainingResult
{
    CompositionNeuralTrainingResult training;
    CompositionNeuralModel model;
    CompositionNeuralModelArtifact artifact;

    bool valid = false;

    bool isValid() const noexcept;
};

CompositionNeuralArtifactTrainingResult
trainCompositionNeuralArtifact(
    const CompositionDatasetPreparedView& prepared,
    const CompositionNeuralTrainingConfig& config) noexcept;

} // namespace midigengx::music
