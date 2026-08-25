#include "CompositionNeuralArtifactTrainingService.h"

#include <cmath>

namespace midigengx::music
{

bool CompositionNeuralArtifactTrainingResult::isValid()
    const noexcept
{
    return valid &&
           training.isValid() &&
           model.isValid() &&
           artifact.isValid();
}

CompositionNeuralArtifactTrainingResult
trainCompositionNeuralArtifact(
    const CompositionDatasetPreparedView& prepared,
    const CompositionNeuralTrainingConfig& config)
    noexcept
{
    CompositionNeuralArtifactTrainingResult result;

    if (!prepared.isValid() ||
        config.epochs == 0 ||
        config.learningRate <= 0.0)
    {
        return result;
    }

    const auto contract =
        buildCompositionLearningContract(
            prepared,
            LearningObjective::NextSectionPrediction);

    if (!contract.isValid())
        return result;

    auto model =
        initializeCompositionNeuralModel(
            contract);

    if (!model.isValid())
        return result;

    const auto training =
        trainCompositionNeuralModel(
            model,
            prepared,
            config);

    if (!training.isValid() ||
        !training.trained ||
        training.epochsCompleted == 0 ||
        !std::isfinite(training.initialLoss) ||
        !std::isfinite(training.finalLoss))
    {
        return result;
    }

    const auto artifact =
        serializeCompositionNeuralModel(
            model);

    if (!artifact.isValid())
        return result;

    result.training =
        training;
    result.model =
        std::move(model);
    result.artifact =
        std::move(artifact);
    result.valid = true;

    return result;
}

} // namespace midigengx::music
