#include "CompositionSequenceNeuralTrainer.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace midigengx::music
{
namespace
{

struct Gradients
{
    std::vector<double> inputWeights;
    std::vector<double> recurrentWeights;
    std::vector<double> hiddenBias;
    std::vector<double> outputWeights;
    std::vector<double> outputBias;
};

bool finite(
    double value) noexcept
{
    return std::isfinite(value);
}

double clipped(
    double value,
    double clip) noexcept
{
    return std::clamp(
        value,
        -clip,
        clip);
}

Gradients makeGradients(
    const CompositionSequenceNeuralModel& model)
{
    Gradients gradients;

    gradients.inputWeights.assign(
        model.inputWeights.size(),
        0.0);

    gradients.recurrentWeights.assign(
        model.recurrentWeights.size(),
        0.0);

    gradients.hiddenBias.assign(
        model.hiddenBias.size(),
        0.0);

    gradients.outputWeights.assign(
        model.outputWeights.size(),
        0.0);

    gradients.outputBias.assign(
        model.outputBias.size(),
        0.0);

    return gradients;
}

double trainWindow(
    CompositionSequenceNeuralModel& model,
    const CompositionMidiSequenceWindow& window,
    const CompositionSequenceNeuralTrainingConfig& config,
    Gradients& gradients) noexcept
{
    const auto width =
        model.contract.inputFeatureWidth;

    const auto targetWidth =
        model.contract.targetFeatureWidth;

    const auto timeCount =
        model.contract.contextLength;

    std::vector<std::vector<double>> hidden(
        timeCount + 1,
        std::vector<double>(
            CompositionSequenceNeuralModel::hiddenWidth,
            0.0));

    std::vector<double> active(
        timeCount,
        0.0);

    for (std::size_t time = 0;
         time < timeCount;
         ++time)
    {
        active[time] =
            window.paddingMask[time];

        if (active[time] <= 0.0)
            continue;

        const auto inputBase =
            time * width;

        for (std::size_t row = 0;
             row < CompositionSequenceNeuralModel::hiddenWidth;
             ++row)
        {
            double sum =
                model.hiddenBias[row];

            for (std::size_t column = 0;
                 column < width;
                 ++column)
            {
                sum +=
                    model.inputWeights[
                        row * width +
                        column] *
                    window.inputs[
                        inputBase +
                        column];
            }

            const auto recurrentBase =
                row *
                CompositionSequenceNeuralModel::hiddenWidth;

            for (std::size_t column = 0;
                 column < CompositionSequenceNeuralModel::hiddenWidth;
                 ++column)
            {
                sum +=
                    model.recurrentWeights[
                        recurrentBase +
                        column] *
                    hidden[time][column];
            }

            hidden[time + 1][row] =
                std::tanh(sum);
        }
    }

    std::vector<double> output(
        targetWidth,
        0.0);

    for (std::size_t row = 0;
         row < targetWidth;
         ++row)
    {
        double sum =
            model.outputBias[row];

        const auto base =
            row *
            CompositionSequenceNeuralModel::hiddenWidth;

        for (std::size_t column = 0;
             column < CompositionSequenceNeuralModel::hiddenWidth;
             ++column)
        {
            sum +=
                model.outputWeights[
                    base + column] *
                hidden[timeCount][column];
        }

        output[row] =
            std::clamp(
                sum,
                -1.0,
                1.0);
    }

    double loss = 0.0;

    std::vector<double> dhNext(
        CompositionSequenceNeuralModel::hiddenWidth,
        0.0);

    for (std::size_t row = 0;
         row < targetWidth;
         ++row)
    {
        const auto error =
            output[row] -
            window.targets[row];

        loss +=
            error * error;

        const auto base =
            row *
            CompositionSequenceNeuralModel::hiddenWidth;

        for (std::size_t column = 0;
             column < CompositionSequenceNeuralModel::hiddenWidth;
             ++column)
        {
            gradients.outputWeights[
                base + column] +=
                error *
                hidden[timeCount][column];
        }

        gradients.outputBias[row] +=
            error;
    }

    // Output gradient scale for mean squared error.
    loss /=
        static_cast<double>(
            targetWidth);

    for (std::size_t row = 0;
         row < targetWidth;
         ++row)
    {
        const auto error =
            (output[row] -
             window.targets[row]) *
            (2.0 /
             static_cast<double>(
                 targetWidth));

        const auto base =
            row *
            CompositionSequenceNeuralModel::hiddenWidth;

        for (std::size_t column = 0;
             column < CompositionSequenceNeuralModel::hiddenWidth;
             ++column)
        {
            dhNext[column] +=
                error *
                model.outputWeights[
                    base + column];
        }

        gradients.outputBias[row] +=
            error;
    }

    for (std::size_t reverse = timeCount;
         reverse > 0;
         --reverse)
    {
        const auto time =
            reverse - 1;

        if (active[time] <= 0.0)
        {
            dhNext.assign(
                dhNext.size(),
                0.0);
            continue;
        }

        std::vector<double> dh(
            CompositionSequenceNeuralModel::hiddenWidth,
            0.0);

        for (std::size_t row = 0;
             row < CompositionSequenceNeuralModel::hiddenWidth;
             ++row)
        {
            const auto derivative =
                1.0 -
                hidden[time + 1][row] *
                hidden[time + 1][row];

            dh[row] =
                dhNext[row] *
                derivative;
        }

        const auto inputBase =
            time * width;

        for (std::size_t row = 0;
             row < CompositionSequenceNeuralModel::hiddenWidth;
             ++row)
        {
            gradients.hiddenBias[row] +=
                dh[row];

            for (std::size_t column = 0;
                 column < width;
                 ++column)
            {
                gradients.inputWeights[
                    row * width +
                    column] +=
                    dh[row] *
                    window.inputs[
                        inputBase +
                        column];
            }

            for (std::size_t column = 0;
                 column < CompositionSequenceNeuralModel::hiddenWidth;
                 ++column)
            {
                gradients.recurrentWeights[
                    row *
                        CompositionSequenceNeuralModel::hiddenWidth +
                    column] +=
                    dh[row] *
                    hidden[time][column];
            }
        }

        std::vector<double> nextDh(
            CompositionSequenceNeuralModel::hiddenWidth,
            0.0);

        for (std::size_t row = 0;
             row < CompositionSequenceNeuralModel::hiddenWidth;
             ++row)
        {
            const auto base =
                row *
                CompositionSequenceNeuralModel::hiddenWidth;

            for (std::size_t column = 0;
                 column < CompositionSequenceNeuralModel::hiddenWidth;
                 ++column)
            {
                nextDh[column] +=
                    dh[row] *
                    model.recurrentWeights[
                        base + column];
            }
        }

        dhNext =
            std::move(nextDh);
    }

    return loss;
}

void clipGradients(
    Gradients& gradients,
    double clip)
{
    for (auto& value :
         gradients.inputWeights)
        value = clipped(value, clip);

    for (auto& value :
         gradients.recurrentWeights)
        value = clipped(value, clip);

    for (auto& value :
         gradients.hiddenBias)
        value = clipped(value, clip);

    for (auto& value :
         gradients.outputWeights)
        value = clipped(value, clip);

    for (auto& value :
         gradients.outputBias)
        value = clipped(value, clip);
}

void applyGradients(
    CompositionSequenceNeuralModel& model,
    const Gradients& gradients,
    double learningRate)
{
    for (std::size_t index = 0;
         index < model.inputWeights.size();
         ++index)
    {
        model.inputWeights[index] -=
            learningRate *
            gradients.inputWeights[index];
    }

    for (std::size_t index = 0;
         index < model.recurrentWeights.size();
         ++index)
    {
        model.recurrentWeights[index] -=
            learningRate *
            gradients.recurrentWeights[index];
    }

    for (std::size_t index = 0;
         index < model.hiddenBias.size();
         ++index)
    {
        model.hiddenBias[index] -=
            learningRate *
            gradients.hiddenBias[index];
    }

    for (std::size_t index = 0;
         index < model.outputWeights.size();
         ++index)
    {
        model.outputWeights[index] -=
            learningRate *
            gradients.outputWeights[index];
    }

    for (std::size_t index = 0;
         index < model.outputBias.size();
         ++index)
    {
        model.outputBias[index] -=
            learningRate *
            gradients.outputBias[index];
    }
}

} // namespace

bool CompositionSequenceNeuralTrainingResult::isValid()
    const noexcept
{
    return trained &&
           epochsCompleted > 0 &&
           windowCount > 0 &&
           finite(initialLoss) &&
           finite(finalLoss);
}

CompositionSequenceNeuralTrainingResult
trainCompositionSequenceNeuralModel(
    CompositionSequenceNeuralModel& model,
    const std::vector<CompositionMidiTrainingSequence>& sequences,
    const CompositionSequenceNeuralTrainingConfig& config)
    noexcept
{
    CompositionSequenceNeuralTrainingResult result;

    if (!model.isValid() ||
        sequences.empty() ||
        config.epochs == 0 ||
        !finite(config.learningRate) ||
        config.learningRate <= 0.0 ||
        !finite(config.gradientClip) ||
        config.gradientClip <= 0.0)
    {
        return result;
    }

    std::vector<CompositionMidiSequenceWindow> windows;

    for (const auto& sequence :
         sequences)
    {
        const auto sequenceWindows =
            buildCompositionMidiSequenceWindows(
                sequence,
                model.contract);

        windows.insert(
            windows.end(),
            sequenceWindows.begin(),
            sequenceWindows.end());
    }

    if (windows.empty())
        return result;

    result.windowCount =
        windows.size();

    Gradients gradients =
        makeGradients(
            model);

    auto evaluateLoss =
        [&]()
        {
            double total = 0.0;

            for (const auto& window :
                 windows)
            {
                const auto prediction =
                    model.predictNextEvent(
                        window);

                if (!prediction.isValid(
                        model.contract.targetFeatureWidth))
                {
                    return std::numeric_limits<double>::infinity();
                }

                for (std::size_t index = 0;
                     index <
                         prediction.features.size();
                     ++index)
                {
                    const auto error =
                        prediction.features[index] -
                        window.targets[index];

                    total +=
                        error * error;
                }
            }

            return total /
                   static_cast<double>(
                       windows.size() *
                       model.contract.targetFeatureWidth);
        };

    result.initialLoss =
        evaluateLoss();

    if (!finite(result.initialLoss))
        return result;

    for (std::size_t epoch = 0;
         epoch < config.epochs;
         ++epoch)
    {
        std::fill(
            gradients.inputWeights.begin(),
            gradients.inputWeights.end(),
            0.0);

        std::fill(
            gradients.recurrentWeights.begin(),
            gradients.recurrentWeights.end(),
            0.0);

        std::fill(
            gradients.hiddenBias.begin(),
            gradients.hiddenBias.end(),
            0.0);

        std::fill(
            gradients.outputWeights.begin(),
            gradients.outputWeights.end(),
            0.0);

        std::fill(
            gradients.outputBias.begin(),
            gradients.outputBias.end(),
            0.0);

        double epochLoss =
            0.0;

        for (const auto& window :
             windows)
        {
            epochLoss +=
                trainWindow(
                    model,
                    window,
                    config,
                    gradients);
        }

        const auto scale =
            1.0 /
            static_cast<double>(
                windows.size());

        for (auto& value :
             gradients.inputWeights)
            value *= scale;

        for (auto& value :
             gradients.recurrentWeights)
            value *= scale;

        for (auto& value :
             gradients.hiddenBias)
            value *= scale;

        for (auto& value :
             gradients.outputWeights)
            value *= scale;

        for (auto& value :
             gradients.outputBias)
            value *= scale;

        clipGradients(
            gradients,
            config.gradientClip);

        applyGradients(
            model,
            gradients,
            config.learningRate);

        if (!model.isValid())
            return result;

        ++result.epochsCompleted;
    }

    result.finalLoss =
        evaluateLoss();

    result.trained =
        finite(result.finalLoss) &&
        result.epochsCompleted ==
            config.epochs;

    return result;
}

} // namespace midigengx::music
