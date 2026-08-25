#include "CompositionConditionedSequenceNeuralTrainer.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

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

    std::vector<double> composerEmbeddings;
    std::vector<double> styleEmbeddings;
    std::vector<double> eraEmbeddings;
    std::vector<double> instrumentationEmbeddings;
};

bool finiteValue(double value) noexcept
{
    return std::isfinite(value);
}

double clipValue(
    double value,
    double clip) noexcept
{
    return std::clamp(
        value,
        -clip,
        clip);
}

Gradients makeGradients(
    const CompositionConditionedSequenceNeuralModel& model)
{
    Gradients g;

    g.inputWeights.assign(
        model.inputWeights.size(),
        0.0);

    g.recurrentWeights.assign(
        model.recurrentWeights.size(),
        0.0);

    g.hiddenBias.assign(
        model.hiddenBias.size(),
        0.0);

    g.outputWeights.assign(
        model.outputWeights.size(),
        0.0);

    g.outputBias.assign(
        model.outputBias.size(),
        0.0);

    g.composerEmbeddings.assign(
        model.composerEmbeddings.size(),
        0.0);

    g.styleEmbeddings.assign(
        model.styleEmbeddings.size(),
        0.0);

    g.eraEmbeddings.assign(
        model.eraEmbeddings.size(),
        0.0);

    g.instrumentationEmbeddings.assign(
        model.instrumentationEmbeddings.size(),
        0.0);

    return g;
}

void addEmbeddingGradient(
    std::vector<double>& gradient,
    std::uint32_t categoryIndex,
    const std::vector<double>& hiddenGradient)
{
    constexpr auto width =
        CompositionConditionedSequenceNeuralModel::conditionEmbeddingWidth;

    const auto base =
        static_cast<std::size_t>(
            categoryIndex) *
        width;

    for (std::size_t index = 0;
         index < width &&
         base + index < gradient.size();
         ++index)
    {
        gradient[
            base + index] +=
            hiddenGradient[index];
    }
}

double trainWindow(
    CompositionConditionedSequenceNeuralModel& model,
    const CompositionMidiSequenceWindow& window,
    const CompositionConditionedTrainingSample& sample,
    Gradients& gradients)
{
    const auto width =
        model.contract.inputFeatureWidth;

    const auto targetWidth =
        model.contract.targetFeatureWidth;

    const auto timeCount =
        model.contract.contextLength;

    using HiddenVector =
        std::vector<double>;

    std::vector<HiddenVector> hidden(
        timeCount + 1,
        HiddenVector(
            CompositionConditionedSequenceNeuralModel::hiddenWidth,
            0.0));

    const auto embeddingWidth =
        CompositionConditionedSequenceNeuralModel::conditionEmbeddingWidth;

    auto addInitialCondition =
        [&](HiddenVector& target)
        {
            const auto add =
                [&](const std::vector<double>& embedding,
                    std::uint32_t index)
                {
                    const auto base =
                        static_cast<std::size_t>(
                            index) *
                        embeddingWidth;

                    for (std::size_t dimension = 0;
                         dimension < embeddingWidth;
                         ++dimension)
                    {
                        target[dimension] +=
                            embedding[
                                base + dimension];
                    }
                };

            add(
                model.composerEmbeddings,
                sample.composerIndex);

            add(
                model.styleEmbeddings,
                sample.styleIndex);

            add(
                model.eraEmbeddings,
                sample.eraIndex);

            add(
                model.instrumentationEmbeddings,
                sample.instrumentationIndex);
        };

    addInitialCondition(
        hidden[0]);

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
             row < CompositionConditionedSequenceNeuralModel::hiddenWidth;
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
                CompositionConditionedSequenceNeuralModel::hiddenWidth;

            for (std::size_t column = 0;
                 column < CompositionConditionedSequenceNeuralModel::hiddenWidth;
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
            CompositionConditionedSequenceNeuralModel::hiddenWidth;

        for (std::size_t column = 0;
             column < CompositionConditionedSequenceNeuralModel::hiddenWidth;
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
        CompositionConditionedSequenceNeuralModel::hiddenWidth,
        0.0);

    const auto outputScale =
        2.0 /
        static_cast<double>(
            targetWidth);

    for (std::size_t row = 0;
         row < targetWidth;
         ++row)
    {
        const auto rawError =
            output[row] -
            window.targets[row];

        loss +=
            rawError * rawError;

        const auto gradient =
            rawError *
            outputScale;

        const auto base =
            row *
            CompositionConditionedSequenceNeuralModel::hiddenWidth;

        for (std::size_t column = 0;
             column < CompositionConditionedSequenceNeuralModel::hiddenWidth;
             ++column)
        {
            gradients.outputWeights[
                base + column] +=
                gradient *
                hidden[timeCount][column];

            dhNext[column] +=
                gradient *
                model.outputWeights[
                    base + column];
        }

        gradients.outputBias[row] +=
            gradient;
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
            CompositionConditionedSequenceNeuralModel::hiddenWidth,
            0.0);

        for (std::size_t row = 0;
             row < CompositionConditionedSequenceNeuralModel::hiddenWidth;
             ++row)
        {
            dh[row] =
                dhNext[row] *
                (1.0 -
                 hidden[time + 1][row] *
                 hidden[time + 1][row]);
        }

        const auto inputBase =
            time * width;

        for (std::size_t row = 0;
             row < CompositionConditionedSequenceNeuralModel::hiddenWidth;
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
                 column < CompositionConditionedSequenceNeuralModel::hiddenWidth;
                 ++column)
            {
                gradients.recurrentWeights[
                    row *
                        CompositionConditionedSequenceNeuralModel::hiddenWidth +
                    column] +=
                    dh[row] *
                    hidden[time][column];
            }
        }

        if (time == 0)
        {
            addEmbeddingGradient(
                gradients.composerEmbeddings,
                sample.composerIndex,
                dh);

            addEmbeddingGradient(
                gradients.styleEmbeddings,
                sample.styleIndex,
                dh);

            addEmbeddingGradient(
                gradients.eraEmbeddings,
                sample.eraIndex,
                dh);

            addEmbeddingGradient(
                gradients.instrumentationEmbeddings,
                sample.instrumentationIndex,
                dh);
        }

        std::vector<double> nextDh(
            CompositionConditionedSequenceNeuralModel::hiddenWidth,
            0.0);

        for (std::size_t row = 0;
             row < CompositionConditionedSequenceNeuralModel::hiddenWidth;
             ++row)
        {
            const auto base =
                row *
                CompositionConditionedSequenceNeuralModel::hiddenWidth;

            for (std::size_t column = 0;
                 column < CompositionConditionedSequenceNeuralModel::hiddenWidth;
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

    return loss /
           static_cast<double>(
               targetWidth);
}

template <typename Vector>
void clipVector(
    Vector& values,
    double clip)
{
    for (auto& value :
         values)
    {
        value =
            clipValue(
                value,
                clip);
    }
}

void clipGradients(
    Gradients& g,
    double clip)
{
    clipVector(g.inputWeights, clip);
    clipVector(g.recurrentWeights, clip);
    clipVector(g.hiddenBias, clip);
    clipVector(g.outputWeights, clip);
    clipVector(g.outputBias, clip);
    clipVector(g.composerEmbeddings, clip);
    clipVector(g.styleEmbeddings, clip);
    clipVector(g.eraEmbeddings, clip);
    clipVector(g.instrumentationEmbeddings, clip);
}

template <typename Vector>
void applyVector(
    Vector& parameters,
    const Vector& gradients,
    double learningRate)
{
    for (std::size_t index = 0;
         index < parameters.size();
         ++index)
    {
        parameters[index] -=
            learningRate *
            gradients[index];
    }
}

void applyGradients(
    CompositionConditionedSequenceNeuralModel& model,
    const Gradients& g,
    double learningRate)
{
    applyVector(
        model.inputWeights,
        g.inputWeights,
        learningRate);

    applyVector(
        model.recurrentWeights,
        g.recurrentWeights,
        learningRate);

    applyVector(
        model.hiddenBias,
        g.hiddenBias,
        learningRate);

    applyVector(
        model.outputWeights,
        g.outputWeights,
        learningRate);

    applyVector(
        model.outputBias,
        g.outputBias,
        learningRate);

    applyVector(
        model.composerEmbeddings,
        g.composerEmbeddings,
        learningRate);

    applyVector(
        model.styleEmbeddings,
        g.styleEmbeddings,
        learningRate);

    applyVector(
        model.eraEmbeddings,
        g.eraEmbeddings,
        learningRate);

    applyVector(
        model.instrumentationEmbeddings,
        g.instrumentationEmbeddings,
        learningRate);
}

} // namespace

bool CompositionConditionedSequenceNeuralTrainingResult::isValid()
    const noexcept
{
    return trained &&
           epochsCompleted > 0 &&
           windowCount > 0 &&
           finiteValue(initialLoss) &&
           finiteValue(finalLoss);
}

CompositionConditionedSequenceNeuralTrainingResult
trainCompositionConditionedSequenceNeuralModel(
    CompositionConditionedSequenceNeuralModel& model,
    const CompositionConditionedTrainingDataset& dataset,
    const CompositionConditionedSequenceNeuralTrainingConfig& config)
    noexcept
{
    CompositionConditionedSequenceNeuralTrainingResult result;

    if (!model.isValid() ||
        !dataset.isValid() ||
        config.epochs == 0 ||
        !finiteValue(config.learningRate) ||
        config.learningRate <= 0.0 ||
        !finiteValue(config.gradientClip) ||
        config.gradientClip <= 0.0)
    {
        return result;
    }

    struct WindowSamplePair
    {
        CompositionMidiSequenceWindow window;
        const CompositionConditionedTrainingSample* sample = nullptr;
    };

    std::vector<WindowSamplePair> examples;

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
            examples.push_back(
            {
                window,
                &sample
            });
        }
    }

    if (examples.empty())
        return result;

    result.windowCount =
        examples.size();

    Gradients gradients =
        makeGradients(
            model);

    auto evaluateLoss =
        [&]()
        {
            double total = 0.0;

            for (const auto& example :
                 examples)
            {
                const auto prediction =
                    model.predictNextEvent(
                        example.window,
                        *example.sample);

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
                        example.window.targets[index];

                    total +=
                        error * error;
                }
            }

            return total /
                   static_cast<double>(
                       examples.size() *
                       model.contract.targetFeatureWidth);
        };

    result.initialLoss =
        evaluateLoss();

    if (!finiteValue(result.initialLoss))
        return result;

    for (std::size_t epoch = 0;
         epoch < config.epochs;
         ++epoch)
    {
        for (auto* values :
             {
                 &gradients.inputWeights,
                 &gradients.recurrentWeights,
                 &gradients.hiddenBias,
                 &gradients.outputWeights,
                 &gradients.outputBias,
                 &gradients.composerEmbeddings,
                 &gradients.styleEmbeddings,
                 &gradients.eraEmbeddings,
                 &gradients.instrumentationEmbeddings
             })
        {
            std::fill(
                values->begin(),
                values->end(),
                0.0);
        }

        for (const auto& example :
             examples)
        {
            trainWindow(
                model,
                example.window,
                *example.sample,
                gradients);
        }

        const auto scale =
            1.0 /
            static_cast<double>(
                examples.size());

        for (auto* values :
             {
                 &gradients.inputWeights,
                 &gradients.recurrentWeights,
                 &gradients.hiddenBias,
                 &gradients.outputWeights,
                 &gradients.outputBias,
                 &gradients.composerEmbeddings,
                 &gradients.styleEmbeddings,
                 &gradients.eraEmbeddings,
                 &gradients.instrumentationEmbeddings
             })
        {
            for (auto& value :
                 *values)
            {
                value *= scale;
            }
        }

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
        finiteValue(result.finalLoss) &&
        result.epochsCompleted ==
            config.epochs;

    return result;
}

} // namespace midigengx::music
