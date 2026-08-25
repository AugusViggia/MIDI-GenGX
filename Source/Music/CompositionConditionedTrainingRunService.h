#pragma once

#include "CompositionConditionedTrainingDatasetArtifact.h"
#include "CompositionMidiTrainingCorpusArtifact.h"
#include "CompositionConditionedSequenceNeuralTrainingService.h"
#include "CompositionSequenceMetadataArtifact.h"

namespace midigengx::music
{

struct CompositionConditionedTrainingRunResult
{
    CompositionConditionedTrainingDatasetArtifact datasetArtifact;
    CompositionConditionedSequenceNeuralTrainingResult training;
    CompositionConditionedSequenceNeuralModelArtifact modelArtifact;

    std::size_t sequenceCount = 0;
    std::size_t conditionedSampleCount = 0;

    bool valid = false;

    bool isValid() const noexcept;
};

CompositionConditionedTrainingRunResult
runCompositionConditionedTraining(
    const CompositionMidiTrainingCorpusArtifact& sequenceCorpusArtifact,
    const CompositionSequenceMetadataArtifact& metadataArtifact,
    const CompositionConditionedSequenceNeuralTrainingConfig& config) noexcept;

} // namespace midigengx::music
