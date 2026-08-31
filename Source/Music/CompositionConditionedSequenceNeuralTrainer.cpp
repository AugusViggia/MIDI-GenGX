
#include "CompositionConditionedSequenceNeuralTrainer.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <thread>
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

double featureLossWeight(
    const std::size_t featureIndex,
    const CompositionConditionedSequenceNeuralTrainingConfig& config) noexcept
{
    if (featureIndex == 0) return config.pitchLossWeight;
    if (featureIndex == 1) return config.velocityLossWeight;
    if (featureIndex == 2 || featureIndex == 3) return config.timingLossWeight;
    return config.auxiliaryLossWeight;
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
    Gradients& gradients,
    const CompositionConditionedSequenceNeuralTrainingConfig& config)
{
    const auto width =
        model.contract.inputFeatureWidth;

    const auto targetWidth =
        model.contract.targetFeatureWidth;

    const auto timeCount =
        model.contract.contextLength;

    if (timeCount == 0 ||
        timeCount > 1024 ||
        width == 0 ||
        targetWidth == 0)
    {
        return std::numeric_limits<double>::infinity();
    }

    struct Workspace
    {
        std::vector<double> hidden;
        std::vector<double> dhNext;
        std::vector<double> dh;
        std::vector<double> nextDh;

        void prepare(
            std::size_t timeCount,
            std::size_t hiddenWidth)
        {
            hidden.resize(
                (timeCount + 1) *
                hiddenWidth);

            dhNext.resize(hiddenWidth);
            dh.resize(hiddenWidth);
            nextDh.resize(hiddenWidth);
        }
    };

    thread_local Workspace workspace;

    workspace.prepare(
        timeCount,
        CompositionConditionedSequenceNeuralModel::hiddenWidth);

    std::fill(
        workspace.hidden.begin(),
        workspace.hidden.end(),
        0.0);

    std::fill(
        workspace.dhNext.begin(),
        workspace.dhNext.end(),
        0.0);

    const auto embeddingWidth =
        CompositionConditionedSequenceNeuralModel::conditionEmbeddingWidth;

    auto addInitialCondition =
        [&](std::vector<double>& target)
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
                         dimension < embeddingWidth &&
                         dimension <
                             CompositionConditionedSequenceNeuralModel::hiddenWidth;
                         ++dimension)
                    {
                        if (base + dimension <
                            embedding.size())
                        {
                            target[dimension] +=
                                embedding[
                                    base + dimension];
                        }
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
        workspace.hidden);

    for (std::size_t time = 0;
         time < timeCount;
         ++time)
    {
        const auto inputBase =
            time * width;

        const auto hiddenBase =
            time *
            CompositionConditionedSequenceNeuralModel::hiddenWidth;

        const auto nextHiddenBase =
            (time + 1) *
            CompositionConditionedSequenceNeuralModel::hiddenWidth;

        for (std::size_t row = 0;
             row <
                 CompositionConditionedSequenceNeuralModel::hiddenWidth;
             ++row)
        {
            double sum =
                model.hiddenBias[row];

            const auto inputWeightBase =
                row * width;

            for (std::size_t column = 0;
                 column < width;
                 ++column)
            {
                sum +=
                    model.inputWeights[
                        inputWeightBase +
                        column] *
                    window.inputs[
                        inputBase +
                        column];
            }

            const auto recurrentBase =
                row *
                CompositionConditionedSequenceNeuralModel::hiddenWidth;

            for (std::size_t column = 0;
                 column <
                     CompositionConditionedSequenceNeuralModel::hiddenWidth;
                 ++column)
            {
                sum +=
                    model.recurrentWeights[
                        recurrentBase +
                        column] *
                    workspace.hidden[
                        hiddenBase +
                        column];
            }

            workspace.hidden[
                nextHiddenBase +
                row] =
                std::tanh(sum);
        }
    }

    double loss = 0.0;

    const auto finalHiddenBase =
        timeCount *
        CompositionConditionedSequenceNeuralModel::hiddenWidth;

    const auto outputScale =
        2.0 /
        static_cast<double>(
            targetWidth);

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
             column <
                 CompositionConditionedSequenceNeuralModel::hiddenWidth;
             ++column)
        {
            sum +=
                model.outputWeights[
                    base +
                    column] *
                workspace.hidden[
                    finalHiddenBase +
                    column];
        }

        const auto output =
            std::clamp(
                sum,
                -1.0,
                1.0);

        const auto error =
            output -
            window.targets[row];

        const auto weight = featureLossWeight(row, config);

        loss +=
            weight * error * error;

        const auto gradient =
            weight * error * outputScale;

        for (std::size_t column = 0;
             column <
                 CompositionConditionedSequenceNeuralModel::hiddenWidth;
             ++column)
        {
            gradients.outputWeights[
                base +
                column] +=
                gradient *
                workspace.hidden[
                    finalHiddenBase +
                    column];

            workspace.dhNext[column] +=
                gradient *
                model.outputWeights[
                    base +
                    column];
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

        const auto inputBase =
            time * width;

        const auto hiddenBase =
            time *
            CompositionConditionedSequenceNeuralModel::hiddenWidth;

        const auto nextHiddenBase =
            (time + 1) *
            CompositionConditionedSequenceNeuralModel::hiddenWidth;

        for (std::size_t row = 0;
             row <
                 CompositionConditionedSequenceNeuralModel::hiddenWidth;
             ++row)
        {
            workspace.dh[row] =
                workspace.dhNext[row] *
                (1.0 -
                 workspace.hidden[
                     nextHiddenBase +
                     row] *
                 workspace.hidden[
                     nextHiddenBase +
                     row]);
        }

        for (std::size_t row = 0;
             row <
                 CompositionConditionedSequenceNeuralModel::hiddenWidth;
             ++row)
        {
            const auto gradient =
                workspace.dh[row];

            gradients.hiddenBias[row] +=
                gradient;

            const auto inputWeightBase =
                row * width;

            for (std::size_t column = 0;
                 column < width;
                 ++column)
            {
                gradients.inputWeights[
                    inputWeightBase +
                    column] +=
                    gradient *
                    window.inputs[
                        inputBase +
                        column];
            }

            const auto recurrentBase =
                row *
                CompositionConditionedSequenceNeuralModel::hiddenWidth;

            for (std::size_t column = 0;
                 column <
                     CompositionConditionedSequenceNeuralModel::hiddenWidth;
                 ++column)
            {
                gradients.recurrentWeights[
                    recurrentBase +
                    column] +=
                    gradient *
                    workspace.hidden[
                        hiddenBase +
                        column];
            }
        }

        if (time == 0)
        {
            addEmbeddingGradient(
                gradients.composerEmbeddings,
                sample.composerIndex,
                workspace.dh);

            addEmbeddingGradient(
                gradients.styleEmbeddings,
                sample.styleIndex,
                workspace.dh);

            addEmbeddingGradient(
                gradients.eraEmbeddings,
                sample.eraIndex,
                workspace.dh);

            addEmbeddingGradient(
                gradients.instrumentationEmbeddings,
                sample.instrumentationIndex,
                workspace.dh);
        }

        std::fill(
            workspace.nextDh.begin(),
            workspace.nextDh.end(),
            0.0);

        for (std::size_t row = 0;
             row <
                 CompositionConditionedSequenceNeuralModel::hiddenWidth;
             ++row)
        {
            const auto base =
                row *
                CompositionConditionedSequenceNeuralModel::hiddenWidth;

            for (std::size_t column = 0;
                 column <
                     CompositionConditionedSequenceNeuralModel::hiddenWidth;
                 ++column)
            {
                workspace.nextDh[column] +=
                    workspace.dh[row] *
                    model.recurrentWeights[
                        base +
                        column];
            }
        }

        workspace.dhNext.swap(
            workspace.nextDh);
    }

    double totalWeight = 0.0;
    for (std::size_t row = 0; row < targetWidth; ++row)
        totalWeight += featureLossWeight(row, config);

    return loss / std::max(totalWeight, 1.0);
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

double CompositionConditionedSequenceNeuralTrainingConfig::lossWeightForFeature(
    const std::size_t featureIndex) const noexcept
{
    if (featureIndex == 0)
        return pitchLossWeight;

    if (featureIndex == 1)
        return velocityLossWeight;

    if (featureIndex == 2 || featureIndex == 3)
        return timingLossWeight;

    return auxiliaryLossWeight;
}

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
        config.gradientClip <= 0.0 ||
        config.evaluationInterval == 0 ||
        !finiteValue(config.pitchLossWeight) ||
        !finiteValue(config.timingLossWeight) ||
        !finiteValue(config.velocityLossWeight) ||
        !finiteValue(config.auxiliaryLossWeight) ||
        config.pitchLossWeight <= 0.0 ||
        config.timingLossWeight <= 0.0 ||
        config.velocityLossWeight <= 0.0 ||
        config.auxiliaryLossWeight <= 0.0)
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

            double totalWeight = 0.0;
            for (std::size_t index = 0;
                 index < model.contract.targetFeatureWidth;
                 ++index)
                totalWeight += featureLossWeight(index, config);

            return total /
                   std::max(
                       static_cast<double>(examples.size()) * totalWeight,
                       1.0);
        };

    result.initialLoss =
        evaluateLoss();

    if (!finiteValue(result.initialLoss))
        return result;

    const std::size_t requestedWorkers =
        config.workerThreads == 0
            ? static_cast<std::size_t>(
                  std::thread::hardware_concurrency())
            : config.workerThreads;

    const std::size_t workerCount =
        std::clamp(
            requestedWorkers == 0
                ? std::size_t{1}
                : requestedWorkers,
            std::size_t{1},
            examples.size());

    Gradients gradients =
        makeGradients(model);

    std::vector<Gradients> workerGradients;
    workerGradients.reserve(workerCount);

    for (std::size_t index = 0;
         index < workerCount;
         ++index)
    {
        workerGradients.push_back(
            makeGradients(model));
    }

    for (std::size_t epoch = 0;
         epoch < config.epochs;
         ++epoch)
    {
        for (auto& worker : workerGradients)
        {
            for (auto* values :
                 {
                     &worker.inputWeights,
                     &worker.recurrentWeights,
                     &worker.hiddenBias,
                     &worker.outputWeights,
                     &worker.outputBias,
                     &worker.composerEmbeddings,
                     &worker.styleEmbeddings,
                     &worker.eraEmbeddings,
                     &worker.instrumentationEmbeddings
                 })
            {
                std::fill(
                    values->begin(),
                    values->end(),
                    0.0);
            }
        }

        std::vector<std::thread> workers;
        workers.reserve(workerCount);

        for (std::size_t workerIndex = 0;
             workerIndex < workerCount;
             ++workerIndex)
        {
            workers.emplace_back(
                [&, workerIndex]()
                {
                    const std::size_t begin =
                        examples.size() *
                        workerIndex /
                        workerCount;

                    const std::size_t end =
                        examples.size() *
                        (workerIndex + 1) /
                        workerCount;

                    for (std::size_t exampleIndex = begin;
                         exampleIndex < end;
                         ++exampleIndex)
                    {
                        trainWindow(
                            model,
                            examples[exampleIndex].window,
                            *examples[exampleIndex].sample,
                            workerGradients[workerIndex],
                            config);
                    }
                });
        }

        for (auto& worker :
             workers)
        {
            worker.join();
        }

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

        for (const auto& worker :
             workerGradients)
        {
            auto accumulate =
                [](std::vector<double>& destination,
                   const std::vector<double>& source)
                {
                    for (std::size_t index = 0;
                         index < destination.size();
                         ++index)
                    {
                        destination[index] +=
                            source[index];
                    }
                };

            accumulate(
                gradients.inputWeights,
                worker.inputWeights);
            accumulate(
                gradients.recurrentWeights,
                worker.recurrentWeights);
            accumulate(
                gradients.hiddenBias,
                worker.hiddenBias);
            accumulate(
                gradients.outputWeights,
                worker.outputWeights);
            accumulate(
                gradients.outputBias,
                worker.outputBias);
            accumulate(
                gradients.composerEmbeddings,
                worker.composerEmbeddings);
            accumulate(
                gradients.styleEmbeddings,
                worker.styleEmbeddings);
            accumulate(
                gradients.eraEmbeddings,
                worker.eraEmbeddings);
            accumulate(
                gradients.instrumentationEmbeddings,
                worker.instrumentationEmbeddings);
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

        if (config.progressCallback)
        {
            const auto shouldEvaluate =
                ((epoch + 1) %
                     config.evaluationInterval == 0) ||
                epoch + 1 ==
                    config.epochs;

            const auto epochLoss =
                shouldEvaluate
                    ? evaluateLoss()
                    : std::numeric_limits<double>::quiet_NaN();

            if (shouldEvaluate &&
                !finiteValue(epochLoss))
            {
                return result;
            }

            config.progressCallback(
                result.epochsCompleted,
                config.epochs,
                epochLoss);
        }
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

