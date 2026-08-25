#include "CompositionMidiTrainingEvaluationService.h"

#include "CompositionLearningContract.h"

namespace midigengx::music
{

bool CompositionMidiTrainingEvaluationResult::isValid()
    const noexcept
{
    return valid &&
           training.isValid() &&
           neuralValidation.isValid() &&
           neuralTest.isValid() &&
           baselineValidation.isValid() &&
           baselineTest.isValid() &&
           musicalValidation.isValid() &&
           musicalTest.isValid();
}

bool CompositionMidiTrainingEvaluationResult::neuralBeatsBaselineOnTest()
    const noexcept
{
    if (!neuralTest.isValid() ||
        !baselineTest.isValid())
    {
        return false;
    }

    return neuralTest.meanSquaredError <=
               baselineTest.meanSquaredError &&
           neuralTest.meanAbsoluteError <=
               baselineTest.meanAbsoluteError;
}

CompositionMidiTrainingEvaluationResult
trainAndEvaluateCompositionNeuralModelFromMidiCorpus(
    const std::string& directoryPath,
    bool recursive,
    double validationRatio,
    double testRatio,
    const CompositionNeuralTrainingConfig& config)
    noexcept
{
    CompositionMidiTrainingEvaluationResult result;

    result.training =
        trainCompositionNeuralModelFromMidiCorpus(
            directoryPath,
            recursive,
            validationRatio,
            testRatio,
            config);

    if (!result.training.isValid())
        return result;

    const auto& prepared =
        result.training.pipeline.prepared;

    const auto& model =
        result.training.neuralTraining.model;

    const auto contract =
        buildCompositionLearningContract(
            prepared,
            LearningObjective::NextSectionPrediction);

    if (!contract.isValid())
        return result;

    const auto baseline =
        trainCompositionBaselineModel(
            prepared,
            contract);

    if (!baseline.isValid())
        return result;

    result.neuralValidation =
        evaluateCompositionNeuralModel(
            model,
            prepared,
            true);

    result.neuralTest =
        evaluateCompositionNeuralModel(
            model,
            prepared,
            false);

    result.baselineValidation =
        evaluateCompositionBaselineModel(
            baseline,
            prepared,
            true);

    result.baselineTest =
        evaluateCompositionBaselineModel(
            baseline,
            prepared,
            false);

    result.musicalValidation =
        evaluateCompositionNeuralMusicalQuality(
            model,
            prepared,
            true);

    result.musicalTest =
        evaluateCompositionNeuralMusicalQuality(
            model,
            prepared,
            false);

    result.valid =
        true;

    if (!result.isValid())
        result.valid = false;

    return result;
}

} // namespace midigengx::music
