#include "CompositionConditionedSequenceNeuralEvaluator.h"

#include <cmath>

namespace midigengx::music
{

bool CompositionConditionedSequenceNeuralEvaluationResult::isValid()
    const noexcept
{
    return valid &&
           sampleCount > 0 &&
           windowCount > 0 &&
           std::isfinite(loss) &&
           loss >= 0.0;
}

CompositionConditionedSequenceNeuralEvaluationResult
evaluateCompositionConditionedSequenceNeuralModel(
    const CompositionConditionedSequenceNeuralModel& model,
    const CompositionConditionedTrainingDataset& dataset)
    noexcept
{
    CompositionConditionedSequenceNeuralEvaluationResult result;

    if (!model.isValid() ||
        !dataset.isValid())
    {
        return result;
    }

    double totalLoss = 0.0;
    std::size_t windowCount = 0;

    for (const auto& sample :
         dataset.samples)
    {
        const auto windows =
            buildCompositionMidiSequenceWindows(
                sample.sequence,
                model.contract);

        for (const auto& window :
             windows)
        {
            const auto prediction =
                model.predictNextEvent(
                    window,
                    sample);

            if (!prediction.isValid(
                    model.contract.targetFeatureWidth))
            {
                return result;
            }

            for (std::size_t index = 0;
                 index < prediction.features.size();
                 ++index)
            {
                const auto error =
                    prediction.features[index] -
                    window.targets[index];

                totalLoss +=
                    error * error;
            }

            ++windowCount;
        }
    }

    if (windowCount == 0)
        return result;

    result.sampleCount =
        dataset.samples.size();

    result.windowCount =
        windowCount;

    result.loss =
        totalLoss /
        static_cast<double>(
            windowCount *
            model.contract.targetFeatureWidth);

    result.valid =
        std::isfinite(result.loss);

    return result;
}

} // namespace midigengx::music
