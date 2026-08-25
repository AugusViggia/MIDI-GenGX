#pragma once

#include "CompositionMidiTrainingCorpusArtifact.h"
#include "CompositionSequenceNeuralModelArtifact.h"
#include "CompositionSequenceNeuralTrainer.h"

namespace midigengx::music
{

struct CompositionSequenceNeuralTrainingServiceResult
{
    CompositionSequenceNeuralTrainingResult training;
    CompositionSequenceNeuralModelArtifact artifact;

    bool valid = false;

    bool isValid() const noexcept;
};

CompositionSequenceNeuralTrainingServiceResult
trainSequenceNeuralModelFromArtifact(
    const CompositionMidiTrainingCorpusArtifact& corpusArtifact,
    const CompositionSequenceNeuralTrainingConfig& config) noexcept;

} // namespace midigengx::music
