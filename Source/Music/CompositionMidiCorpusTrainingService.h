#pragma once

#include "CompositionMidiTrainingPipeline.h"
#include "CompositionNeuralArtifactTrainingService.h"

#include <cstddef>
#include <string>

namespace midigengx::music
{

struct CompositionMidiCorpusTrainingResult
{
    CompositionMidiTrainingPipelineResult pipeline;
    CompositionNeuralArtifactTrainingResult neuralTraining;

    bool valid = false;

    bool isValid() const noexcept;

    std::size_t sampleCount() const noexcept;
};

CompositionMidiCorpusTrainingResult
trainCompositionNeuralModelFromMidiCorpus(
    const std::string& directoryPath,
    bool recursive,
    double validationRatio,
    double testRatio,
    const CompositionNeuralTrainingConfig& config) noexcept;

} // namespace midigengx::music
