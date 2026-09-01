#include "CompositionConditionedSequenceNeuralTrainer.h"

#include <algorithm>
#include <cmath>
#include <condition_variable>
#include <limits>
#include <mutex>
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

    g.inputWeights.assign(model.inputWeights.size(), 0.0);
    g.recurrentWeights.assign(model.recurrentWeights.size(), 0.0);
    g.hiddenBias.assign(model.hiddenBias.size(), 0.0);
    g.outputWeights.assign(model.outputWeights.size(), 0.0);
    g.outputBias.assign(model.outputBias.size(), 0.0);
    g.composerEmbeddings.assign(model.composerEmbeddings.size(), 0.0);
    g.styleEmbeddings.assign(model.styleEmbeddings.size(), 0.0);
    g.eraEmbeddings.assign(model.eraEmbeddings.size(), 0.0);
    g.instrumentationEmbeddings.assign(model.instrumentationEmbeddings.size(), 0.0);

    return g;
}

void clearGradients(Gradients& g) noexcept
{
    for (auto* values :
         {
             &g.inputWeights,
             &g.recurrentWeights,
             &g.hiddenBias,
             &g.outputWeights,
             &g.outputBias,
             &g.composerEmbeddings,
             &g.styleEmbeddings,
             &g.eraEmbeddings,
             &g.instrumentationEmbeddings
         })
    {
        std::fill(values->begin(), values->end(), 0.0);
    }
}

void accumulateGradientVectors(
    std::vector<double>& destination,
    const std::vector<double>& source) noexcept
{
    for (std::size_t index = 0;
         index < destination.size();
         ++index)
    {
        destination[index] += source[index];
    }
}

void accumulateGradients(
    Gradients& destination,
    const Gradients& source) noexcept
{
    accumulateGradientVectors(destination.inputWeights, source.inputWeights);
    accumulateGradientVectors(destination.recurrentWeights, source.recurrentWeights);
    accumulateGradientVectors(destination.hiddenBias, source.hiddenBias);
    accumulateGradientVectors(destination.outputWeights, source.outputWeights);
    accumulateGradientVectors(destination.outputBias, source.outputBias);
    accumulateGradientVectors(destination.composerEmbeddings, source.composerEmbeddings);
    accumulateGradientVectors(destination.styleEmbeddings, source.styleEmbeddings);
    accumulateGradientVectors(destination.eraEmbeddings, source.eraEmbeddings);
    accumulateGradientVectors(
        destination.instrumentationEmbeddings,
        source.instrumentationEmbeddings);
}

template <typename Vector>
void applyScaledClippedVector(
    Vector& parameters,
    const Vector& gradients,
    double scale,
    double clip,
    double learningRate)
{
    for (std::size_t index = 0;
         index < parameters.size();
         ++index)
    {
        const auto scaledGradient =
            std::clamp(
                gradients[index] * scale,
                -clip,
                clip);

        parameters[index] -=
            learningRate *
            scaledGradient;
    }
}

void applyScaledClippedGradients(
    CompositionConditionedSequenceNeuralModel& model,
    const Gradients& gradients,
    double scale,
    double clip,
    double learningRate)
{
    applyScaledClippedVector(
        model.inputWeights,
        gradients.inputWeights,
        scale,
        clip,
        learningRate);

    applyScaledClippedVector(
        model.recurrentWeights,
        gradients.recurrentWeights,
        scale,
        clip,
        learningRate);

    applyScaledClippedVector(
        model.hiddenBias,
        gradients.hiddenBias,
        scale,
        clip,
        learningRate);

    applyScaledClippedVector(
        model.outputWeights,
        gradients.outputWeights,
        scale,
        clip,
        learningRate);

    applyScaledClippedVector(
        model.outputBias,
        gradients.outputBias,
        scale,
        clip,
        learningRate);

    applyScaledClippedVector(
        model.composerEmbeddings,
        gradients.composerEmbeddings,
        scale,
        clip,
        learningRate);

    applyScaledClippedVector(
        model.styleEmbeddings,
        gradients.styleEmbeddings,
        scale,
        clip,
        learningRate);

    applyScaledClippedVector(
        model.eraEmbeddings,
        gradients.eraEmbeddings,
        scale,
        clip,
        learningRate);

    applyScaledClippedVector(
        model.instrumentationEmbeddings,
        gradients.instrumentationEmbeddings,
        scale,
        clip,
        learningRate);
}

void addEmbeddingGradient(
    std::vector<double>& gradient,
    std::uint32_t categoryIndex,
    const std::vector<double>& hiddenGradient)
{
    constexpr auto width =
        CompositionConditionedSequenceNeuralModel::conditionEmbeddingWidth;

    const auto base =
        static_cast<std::size_t>(categoryIndex) * width;

    for (std::size_t index = 0;
         index < width;
         ++index)
    {
        gradient[base + index] += hiddenGradient[index];
    }
}

double trainWindow(
    CompositionConditionedSequenceNeuralModel& model,
    const CompositionMidiSequenceWindow& window,
    const CompositionConditionedTrainingSample& sample,
    Gradients& gradients,
    const std::vector<double>& lossWeights,
    const double totalLossWeight)
{
    const auto width =
        model.contract.inputFeatureWidth;

    const auto targetWidth =
        model.contract.targetFeatureWidth;

    const auto timeCount =
        model.contract.contextLength;

    constexpr auto hiddenWidth =
        CompositionConditionedSequenceNeuralModel::hiddenWidth;

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
            hidden.resize((timeCount + 1) * hiddenWidth);
            dhNext.resize(hiddenWidth);
            dh.resize(hiddenWidth);
            nextDh.resize(hiddenWidth);
        }
    };

    thread_local Workspace workspace;

    workspace.prepare(
        timeCount,
        hiddenWidth);

    std::fill(workspace.hidden.begin(), workspace.hidden.end(), 0.0);
    std::fill(workspace.dhNext.begin(), workspace.dhNext.end(), 0.0);

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
                        static_cast<std::size_t>(index) *
                        embeddingWidth;

                    for (std::size_t dimension = 0;
                         dimension < embeddingWidth &&
                         dimension <
                         hiddenWidth;
                         ++dimension)
                    {
                        if (base + dimension < embedding.size())
                        {
                            target[dimension] +=
                                embedding[base + dimension];
                        }
                    }
                };

            add(model.composerEmbeddings, sample.composerIndex);
            add(model.styleEmbeddings, sample.styleIndex);
            add(model.eraEmbeddings, sample.eraIndex);
            add(model.instrumentationEmbeddings, sample.instrumentationIndex);
        };

    addInitialCondition(workspace.hidden);

    for (std::size_t time = 0;
         time < timeCount;
         ++time)
    {
        const auto inputBase =
            time * width;

        const auto hiddenBase =
            time *
            hiddenWidth;

        const auto nextHiddenBase =
            (time + 1) *
            hiddenWidth;

        for (std::size_t row = 0;
             row <
             hiddenWidth;
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
                        inputWeightBase + column] *
                    window.inputs[
                        inputBase + column];
            }

            const auto recurrentBase =
                row *
                hiddenWidth;

            for (std::size_t column = 0;
                 column <
                 hiddenWidth;
                 ++column)
            {
                sum +=
                    model.recurrentWeights[
                        recurrentBase + column] *
                    workspace.hidden[
                        hiddenBase + column];
            }

            workspace.hidden[
                nextHiddenBase + row] =
                std::tanh(sum);
        }
    }

    double loss = 0.0;

    const auto finalHiddenBase =
        timeCount *
        hiddenWidth;

    const auto outputScale =
        2.0 /
        static_cast<double>(targetWidth);

    for (std::size_t row = 0;
         row < targetWidth;
         ++row)
    {
        double sum =
            model.outputBias[row];

        const auto base =
            row *
            hiddenWidth;

        for (std::size_t column = 0;
             column <
             hiddenWidth;
             ++column)
        {
            sum +=
                model.outputWeights[
                    base + column] *
                workspace.hidden[
                    finalHiddenBase + column];
        }

        const auto output =
            std::clamp(sum, -1.0, 1.0);

        const auto error =
            output -
            window.targets[row];

        const auto weight =
            lossWeights[row];

        loss +=
            weight * error * error;

        const auto gradient =
            weight * error * outputScale;

        for (std::size_t column = 0;
             column <
             hiddenWidth;
             ++column)
        {
            gradients.outputWeights[
                base + column] +=
                gradient *
                workspace.hidden[
                    finalHiddenBase + column];

            workspace.dhNext[column] +=
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

        const auto inputBase =
            time * width;

        const auto hiddenBase =
            time *
            hiddenWidth;

        const auto nextHiddenBase =
            (time + 1) *
            hiddenWidth;

        for (std::size_t row = 0;
             row <
             hiddenWidth;
             ++row)
        {
            workspace.dh[row] =
                workspace.dhNext[row] *
                (1.0 -
                 workspace.hidden[
                     nextHiddenBase + row] *
                 workspace.hidden[
                     nextHiddenBase + row]);
        }

        for (std::size_t row = 0;
             row <
             hiddenWidth;
             ++row)
        {
            const auto gradient =
                workspace.dh[row];

            gradients.hiddenBias[row] += gradient;

            const auto inputWeightBase =
                row * width;

            for (std::size_t column = 0;
                 column < width;
                 ++column)
            {
                gradients.inputWeights[
                    inputWeightBase + column] +=
                    gradient *
                    window.inputs[
                        inputBase + column];
            }

            const auto recurrentBase =
                row *
                hiddenWidth;

            for (std::size_t column = 0;
                 column <
                 hiddenWidth;
                 ++column)
            {
                gradients.recurrentWeights[
                    recurrentBase + column] +=
                    gradient *
                    workspace.hidden[
                        hiddenBase + column];
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
             hiddenWidth;
             ++row)
        {
            const auto base =
                row *
                hiddenWidth;

            for (std::size_t column = 0;
                 column <
                 hiddenWidth;
                 ++column)
            {
                workspace.nextDh[column] +=
                    workspace.dh[row] *
                    model.recurrentWeights[
                        base + column];
            }
        }

        workspace.dhNext.swap(
            workspace.nextDh);
    }

    return loss /
           std::max(totalLossWeight, 1.0);
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

    struct ExampleDescriptor
    {
        const CompositionMidiTrainingSequence* sequence = nullptr;
        const CompositionConditionedTrainingSample* sample = nullptr;
        std::size_t targetIndex = 0;
    };

    std::vector<ExampleDescriptor> examples;

    std::size_t totalExampleCount = 0;

    for (const auto& sample :
         dataset.samples)
    {
        if (sample.sequence.events.size() < 2)
            continue;

        totalExampleCount +=
            sample.sequence.events.size() - 1;
    }

    examples.reserve(
        totalExampleCount);

    for (const auto& sample :
         dataset.samples)
    {
        if (sample.sequence.events.size() < 2)
            continue;

        const auto targetCount =
            sample.sequence.events.size() - 1;

        for (std::size_t targetIndex = 1;
             targetIndex <= targetCount;
             ++targetIndex)
        {
            examples.push_back(
            {
                &sample.sequence,
                &sample,
                targetIndex
            });
        }
    }

    if (examples.empty())
        return result;

    result.windowCount =
        examples.size();

    double totalLossWeight = 0.0;
    for (std::size_t index = 0;
         index < model.contract.targetFeatureWidth;
         ++index)
    {
        totalLossWeight +=
            featureLossWeight(index, config);
    }

    const auto normalizedDenominator =
        std::max(
            static_cast<double>(examples.size()) *
                totalLossWeight,
            1.0);

    auto evaluateLoss =
        [&]()
        {
            double total = 0.0;

            CompositionMidiSequenceWindow window;
            CompositionConditionedSequenceNeuralPrediction prediction;

            for (const auto& example :
                 examples)
            {
                if (!buildCompositionMidiSequenceWindowInto(
                        *example.sequence,
                        model.contract,
                        example.targetIndex,
                        window))
                {
                    return std::numeric_limits<double>::infinity();
                }

                if (!model.predictNextEventInto(
                        window,
                        *example.sample,
                        prediction))
                {
                    return std::numeric_limits<double>::infinity();
                }

                for (std::size_t index = 0;
                     index < prediction.features.size();
                     ++index)
                {
                    const auto error =
                        prediction.features[index] -
                        window.targets[index];

                    total +=
                        lossWeights[index] *
                        error *
                        error;
                }
            }

            return total /
                   normalizedDenominator;
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

    std::vector<Gradients> workerGradients;
    workerGradients.reserve(workerCount);

    for (std::size_t index = 0;
         index < workerCount;
         ++index)
    {
        workerGradients.push_back(
            makeGradients(model));
    }

    std::mutex workerMutex;
    std::condition_variable workerCondition;
    std::condition_variable completionCondition;

    std::size_t workerGeneration = 0;
    std::size_t completedWorkers = 0;
    bool stopWorkers = false;

    std::vector<std::thread> workers;
    workers.reserve(workerCount);

    for (std::size_t workerIndex = 0;
         workerIndex < workerCount;
         ++workerIndex)
    {
        workers.emplace_back(
            [&, workerIndex]()
            {
                std::size_t observedGeneration = 0;

                const std::size_t begin =
                    examples.size() *
                    workerIndex /
                    workerCount;

                const std::size_t end =
                    examples.size() *
                    (workerIndex + 1) /
                    workerCount;

                for (;;)
                {
                    std::unique_lock<std::mutex> lock(
                        workerMutex);

                    workerCondition.wait(
                        lock,
                        [&]()
                        {
                            return stopWorkers ||
                                   workerGeneration >
                                       observedGeneration;
                        });

                    if (stopWorkers)
                        return;

                    observedGeneration =
                        workerGeneration;

                    lock.unlock();

                    CompositionMidiSequenceWindow workerWindow;

                    for (std::size_t exampleIndex = begin;
                         exampleIndex < end;
                         ++exampleIndex)
                    {
                        const auto& example =
                            examples[exampleIndex];

                        if (!buildCompositionMidiSequenceWindowInto(
                                *example.sequence,
                                model.contract,
                                example.targetIndex,
                                workerWindow))
                            continue;

                        trainWindow(
                            model,
                            workerWindow,
                            *example.sample,
                            workerGradients[workerIndex],
                            lossWeights,
                            totalLossWeight);
                    }

                    lock.lock();

                    ++completedWorkers;

                    if (completedWorkers ==
                        workerCount)
                    {
                        completionCondition.notify_one();
                    }
                }
            });
    }

    auto stopWorkerPool =
        [&]()
        {
            {
                std::lock_guard<std::mutex> lock(
                    workerMutex);

                stopWorkers = true;
            }

            workerCondition.notify_all();

            for (auto& worker :
                 workers)
            {
                if (worker.joinable())
                    worker.join();
            }
        };

    for (std::size_t epoch = 0;
         epoch < config.epochs;
         ++epoch)
    {
        for (auto& worker :
             workerGradients)
        {
            clearGradients(worker);
        }

        {
            std::lock_guard<std::mutex> lock(
                workerMutex);

            ++workerGeneration;
            completedWorkers = 0;
        }

        workerCondition.notify_all();

        {
            std::unique_lock<std::mutex> lock(
                workerMutex);

            completionCondition.wait(
                lock,
                [&]()
                {
                    return completedWorkers ==
                        workerCount;
                });
        }

        auto& gradients =
            workerGradients.front();

        for (std::size_t workerIndex = 1;
             workerIndex < workerGradients.size();
             ++workerIndex)
        {
            accumulateGradients(
                gradients,
                workerGradients[workerIndex]);
        }

        const auto scale =
            1.0 /
            static_cast<double>(
                examples.size());

        applyScaledClippedGradients(
            model,
            gradients,
            scale,
            config.gradientClip,
            config.learningRate);

        if (!model.isValid())
        {
            stopWorkerPool();
            return result;
        }

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

    stopWorkerPool();

    result.finalLoss =
        evaluateLoss();

    result.trained =
        finiteValue(result.finalLoss) &&
        result.epochsCompleted ==
            config.epochs;

    return result;
}

} // namespace midigengx::music
