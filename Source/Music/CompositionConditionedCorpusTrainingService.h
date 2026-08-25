#pragma once

#include "CompositionConditionedTrainingRunService.h"
#include "CompositionComposerKnowledgeTrainingCorpus.h"
#include "CompositionConditionedSequenceNeuralEvaluator.h"

#include <cstddef>
#include <string>

namespace midigengx::music
{

struct CompositionConditionedCorpusTrainingResult
{
    CompositionMidiTrainingCorpusArtifact sequenceCorpusArtifact;
    CompositionSequenceMetadataArtifact metadataArtifact;
    CompositionConditionedTrainingRunResult trainingRun;

    std::size_t inputFileCount = 0;
    std::size_t sequenceCount = 0;
    std::size_t rejectedFileCount = 0;
    std::size_t trainingSampleCount = 0;
    std::size_t validationSampleCount = 0;
    std::size_t testSampleCount = 0;

    CompositionConditionedSequenceNeuralEvaluationResult
        validationEvaluation;

    CompositionConditionedSequenceNeuralEvaluationResult
        testEvaluation;

    bool valid = false;

    bool isValid() const noexcept;
};

CompositionConditionedCorpusTrainingResult
runCompositionConditionedCorpusTraining(
    const std::string& midiDirectoryPath,
    bool recursive,
    const CompositionSequenceMetadataCatalog& metadataCatalog,
    const CompositionConditionedSequenceNeuralTrainingConfig& config) noexcept;

} // namespace midigengx::music
