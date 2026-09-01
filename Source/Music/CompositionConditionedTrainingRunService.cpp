#include "CompositionConditionedTrainingRunService.h"

#include "CompositionMidiTrainingCorpusArtifact.h"

namespace midigengx::music
{

bool CompositionConditionedTrainingRunResult::isValid()
    const noexcept
{
    return valid &&
           sequenceCount > 0 &&
           conditionedSampleCount ==
               sequenceCount &&
           datasetArtifact.isValid() &&
           training.isValid() &&
           modelArtifact.isValid();
}

CompositionConditionedTrainingRunResult
runCompositionConditionedTrainingFromDataset(
    const CompositionConditionedTrainingDataset& dataset,
    const CompositionMidiTrainingCorpusArtifact& sequenceCorpusArtifact,
    const CompositionSequenceMetadataArtifact& metadataArtifact,
    const CompositionConditionedSequenceNeuralTrainingConfig& config)
    noexcept
{
    CompositionConditionedTrainingRunResult result;

    if (!dataset.isValid() ||
        !sequenceCorpusArtifact.isValid() ||
        !metadataArtifact.isValid())
    {
        return result;
    }

    result.sequenceCount =
        dataset.sampleCount();

    result.conditionedSampleCount =
        dataset.sampleCount();

    result.datasetArtifact =
        serializeCompositionConditionedTrainingDataset(
            dataset);

    if (!result.datasetArtifact.isValid())
        return result;

    const auto training =
        trainCompositionConditionedSequenceNeuralModelFromDataset(
            dataset,
            config);

    if (!training.isValid())
        return result;

    result.training =
        training.training;

    result.modelArtifact =
        training.artifact;

    result.valid =
        true;

    if (!result.isValid())
        result.valid = false;

    return result;
}

CompositionConditionedTrainingRunResult
runCompositionConditionedTraining(
    const CompositionMidiTrainingCorpusArtifact& sequenceCorpusArtifact,
    const CompositionSequenceMetadataArtifact& metadataArtifact,
    const CompositionConditionedSequenceNeuralTrainingConfig& config)
    noexcept
{
    CompositionConditionedTrainingRunResult result;

    if (!sequenceCorpusArtifact.isValid() ||
        !metadataArtifact.isValid())
    {
        return result;
    }

    std::vector<CompositionMidiTrainingSequence>
        sequences;

    if (!deserializeCompositionMidiTrainingSequences(
            sequenceCorpusArtifact,
            sequences) ||
        sequences.empty())
    {
        return result;
    }

    CompositionSequenceMetadataCatalog
        metadataCatalog;

    if (!deserializeCompositionSequenceMetadataCatalog(
            metadataArtifact,
            metadataCatalog) ||
        !metadataCatalog.isValid())
    {
        return result;
    }

    const auto dataset =
        buildCompositionConditionedTrainingDataset(
            sequences,
            metadataCatalog);

    if (!dataset.isValid())
        return result;

    return runCompositionConditionedTrainingFromDataset(
        dataset,
        sequenceCorpusArtifact,
        metadataArtifact,
        config);
}

} // namespace midigengx::music
