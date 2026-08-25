#include "CompositionSequenceNeuralModel.h"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace midigengx::music
{
namespace
{

bool allFinite(
    const std::vector<double>& values) noexcept
{
    for (const auto value :
         values)
    {
        if (!std::isfinite(value))
            return false;
    }

    return true;
}

double tanhSafe(
    double value) noexcept
{
    return std::tanh(value);
}

} // namespace

bool CompositionSequenceNeuralPrediction::isValid(
    std::size_t expectedWidth)
    const noexcept
{
    if (!valid ||
        features.size() != expectedWidth)
    {
        return false;
    }

    for (const auto value :
         features)
    {
        if (!std::isfinite(value) ||
            value < -1.0 ||
            value > 1.0)
        {
            return false;
        }
    }

    return true;
}

bool CompositionSequenceNeuralModel::isValid()
    const noexcept
{
    if (!initialized ||
        !contract.isValid() ||
        inputWeights.size() !=
            hiddenWidth *
            contract.inputFeatureWidth ||
        recurrentWeights.size() !=
            hiddenWidth *
            hiddenWidth ||
        hiddenBias.size() !=
            hiddenWidth ||
        outputWeights.size() !=
            contract.targetFeatureWidth *
            hiddenWidth ||
        outputBias.size() !=
            contract.targetFeatureWidth)
    {
        return false;
    }

    return allFinite(inputWeights) &&
           allFinite(recurrentWeights) &&
           allFinite(hiddenBias) &&
           allFinite(outputWeights) &&
           allFinite(outputBias);
}

CompositionSequenceNeuralModel
initializeCompositionSequenceNeuralModel(
    const CompositionSequenceLearningContract& contract)
    noexcept
{
    CompositionSequenceNeuralModel model;

    if (!contract.isValid())
        return model;

    model.contract =
        contract;

    model.inputWeights.assign(
        CompositionSequenceNeuralModel::hiddenWidth *
            contract.inputFeatureWidth,
        0.0);

    model.recurrentWeights.assign(
        CompositionSequenceNeuralModel::hiddenWidth *
            CompositionSequenceNeuralModel::hiddenWidth,
        0.0);

    model.hiddenBias.assign(
        CompositionSequenceNeuralModel::hiddenWidth,
        0.0);

    model.outputWeights.assign(
        contract.targetFeatureWidth *
            CompositionSequenceNeuralModel::hiddenWidth,
        0.0);

    model.outputBias.assign(
        contract.targetFeatureWidth,
        0.0);

    // Deterministic Xavier-like initialization.
    const auto inputScale =
        1.0 /
        std::sqrt(
            static_cast<double>(
                contract.inputFeatureWidth));

    for (std::size_t row = 0;
         row < CompositionSequenceNeuralModel::hiddenWidth;
         ++row)
    {
        for (std::size_t column = 0;
             column < contract.inputFeatureWidth;
             ++column)
        {
            const auto pattern =
                static_cast<int>(
                    (row * 17 +
                     column * 31) %
                    101) -
                50;

            model.inputWeights[
                row *
                    contract.inputFeatureWidth +
                column] =
                0.05 *
                static_cast<double>(
                    pattern) *
                inputScale;
        }
    }

    const auto recurrentScale =
        0.05 /
        std::sqrt(
            static_cast<double>(
                CompositionSequenceNeuralModel::hiddenWidth));

    for (std::size_t row = 0;
         row < CompositionSequenceNeuralModel::hiddenWidth;
         ++row)
    {
        for (std::size_t column = 0;
             column < CompositionSequenceNeuralModel::hiddenWidth;
             ++column)
        {
            if (row == column)
            {
                model.recurrentWeights[
                    row *
                        CompositionSequenceNeuralModel::hiddenWidth +
                    column] =
                    recurrentScale;
            }
        }
    }

    const auto outputScale =
        0.05 /
        std::sqrt(
            static_cast<double>(
                CompositionSequenceNeuralModel::hiddenWidth));

    for (std::size_t row = 0;
         row < contract.targetFeatureWidth;
         ++row)
    {
        for (std::size_t column = 0;
             column < CompositionSequenceNeuralModel::hiddenWidth;
             ++column)
        {
            const auto pattern =
                static_cast<int>(
                    (row * 13 +
                     column * 7) %
                    29) -
                14;

            model.outputWeights[
                row *
                    CompositionSequenceNeuralModel::hiddenWidth +
                column] =
                static_cast<double>(
                    pattern) *
                outputScale;
        }
    }

    model.initialized =
        true;

    return model;
}

CompositionSequenceNeuralPrediction
CompositionSequenceNeuralModel::predictNextEvent(
    const CompositionMidiSequenceWindow& window)
    const noexcept
{
    CompositionSequenceNeuralPrediction prediction;

    if (!isValid() ||
        !window.isValid(
            contract))
    {
        return prediction;
    }

    const auto width =
        contract.inputFeatureWidth;

    std::vector<double> hidden(
        hiddenWidth,
        0.0);

    for (std::size_t time = 0;
         time < contract.contextLength;
         ++time)
    {
        if (window.paddingMask[time] <= 0.0)
            continue;

        std::vector<double> nextHidden(
            hiddenWidth,
            0.0);

        const auto inputBase =
            time * width;

        for (std::size_t row = 0;
             row < hiddenWidth;
             ++row)
        {
            double sum =
                hiddenBias[row];

            for (std::size_t column = 0;
                 column < width;
                 ++column)
            {
                sum +=
                    inputWeights[
                        row * width +
                        column] *
                    window.inputs[
                        inputBase +
                        column];
            }

            const auto recurrentBase =
                row * hiddenWidth;

            for (std::size_t column = 0;
                 column < hiddenWidth;
                 ++column)
            {
                sum +=
                    recurrentWeights[
                        recurrentBase +
                        column] *
                    hidden[column];
            }

            nextHidden[row] =
                tanhSafe(sum);
        }

        hidden =
            std::move(nextHidden);
    }

    prediction.features.assign(
        contract.targetFeatureWidth,
        0.0);

    for (std::size_t row = 0;
         row < contract.targetFeatureWidth;
         ++row)
    {
        double sum =
            outputBias[row];

        const auto outputBase =
            row * hiddenWidth;

        for (std::size_t column = 0;
             column < hiddenWidth;
             ++column)
        {
            sum +=
                outputWeights[
                    outputBase +
                    column] *
                hidden[column];
        }

        prediction.features[row] =
            std::clamp(
                sum,
                -1.0,
                1.0);
    }

    prediction.valid =
        true;

    return prediction;
}

} // namespace midigengx::music
