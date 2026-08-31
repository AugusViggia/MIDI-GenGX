#include "CompositionConditionedSequenceNeuralEvaluator.h"

#include <cmath>

namespace midigengx::music
{
namespace
{

double featureLossWeight(const std::size_t featureIndex) noexcept
{
    // Keep evaluation aligned with the current structured training loss.
    // Pitch is primary, timing is next, velocity is secondary, and
    // auxiliary features are intentionally down-weighted.
    if (featureIndex == 0)
        return 3.0;

    if (featureIndex == 1)
        return 1.0;

    if (featureIndex == 2 || featureIndex == 3)
        return 2.0;

    return 0.25;
}

} // namespace

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

    double totalWeight = 0.0;
    for (std::size_t index = 0;
         index < model.contract.targetFeatureWidth;
         ++index)
    {
        totalWeight += featureLossWeight(index);
    }

    if (!std::isfinite(totalWeight) ||
        totalWeight <= 0.0)
    {
        return result;
    }

    for (const auto& sample : dataset.samples)
    {
        const auto windows =
            buildCompositionMidiSequenceWindows(
                sample.sequence,
                model.contract);

        for (const auto& window : windows)
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
                    featureLossWeight(index) *
                    error * error;
            }

            ++windowCount;
        }
    }

    if (windowCount == 0)
        return result;

    result.sampleCount = dataset.samples.size();
    result.windowCount = windowCount;
    result.loss =
        totalLoss /
        (static_cast<double>(windowCount) * totalWeight);
    result.valid = std::isfinite(result.loss);

    return result;
}

} // namespace midigengx::music
