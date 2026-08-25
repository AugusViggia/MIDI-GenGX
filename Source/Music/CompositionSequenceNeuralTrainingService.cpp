#include "CompositionSequenceNeuralTrainingService.h"

namespace midigengx::music
{

bool CompositionSequenceNeuralTrainingServiceResult::isValid()
    const noexcept
{
    return valid &&
           training.isValid() &&
           artifact.isValid();
}

CompositionSequenceNeuralTrainingServiceResult
trainSequenceNeuralModelFromArtifact(
    const CompositionMidiTrainingCorpusArtifact& corpusArtifact,
    const CompositionSequenceNeuralTrainingConfig& config)
    noexcept
{
    CompositionSequenceNeuralTrainingServiceResult result;

    if (!corpusArtifact.isValid())
        return result;

    std::vector<CompositionMidiTrainingSequence> sequences;

    if (!deserializeCompositionMidiTrainingSequences(
            corpusArtifact,
            sequences) ||
        sequences.empty())
    {
        return result;
    }

    const auto contract =
        buildCompositionSequenceLearningContract(
            64,
            CompositionSequenceLearningObjective::NextEventPrediction);

    if (!contract.isValid())
        return result;

    auto model =
        initializeCompositionSequenceNeuralModel(
            contract);

    if (!model.isValid())
        return result;

    result.training =
        trainCompositionSequenceNeuralModel(
            model,
            sequences,
            config);

    if (!result.training.isValid())
        return result;

    result.artifact =
        serializeCompositionSequenceNeuralModel(
            model);

    if (!result.artifact.isValid())
        return result;

    result.valid =
        true;

    return result;
}

} // namespace midigengx::music
