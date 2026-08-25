#include "CompositionNeuralTrainer.h"

#include <cmath>
#include <vector>

namespace midigengx::music
{
namespace
{

double squaredError(
    const std::vector<double>& prediction,
    const std::vector<double>& target) noexcept
{
    if (prediction.size() != target.size() ||
        prediction.empty())
    {
        return 0.0;
    }

    double total = 0.0;

    for (std::size_t index = 0;
         index < prediction.size();
         ++index)
    {
        const auto delta =
            prediction[index] -
            target[index];

        total += delta * delta;
    }

    return total /
           static_cast<double>(
               prediction.size());
}

double tanhDerivativeFromOutput(
    double output) noexcept
{
    return 1.0 -
           output * output;
}

struct AdamState
{
    std::vector<double> inputM;
    std::vector<double> inputV;
    std::vector<double> hiddenBiasM;
    std::vector<double> hiddenBiasV;
    std::vector<double> outputM;
    std::vector<double> outputV;
    std::vector<double> outputBiasM;
    std::vector<double> outputBiasV;
    std::vector<double> residualM;
    std::vector<double> residualV;
    std::size_t step = 0;

    void initialize(
        const CompositionNeuralModel& model)
    {
        inputM.assign(model.inputWeights.size(), 0.0);
        inputV.assign(model.inputWeights.size(), 0.0);
        hiddenBiasM.assign(model.hiddenBias.size(), 0.0);
        hiddenBiasV.assign(model.hiddenBias.size(), 0.0);
        outputM.assign(model.outputWeights.size(), 0.0);
        outputV.assign(model.outputWeights.size(), 0.0);
        outputBiasM.assign(model.outputBias.size(), 0.0);
        outputBiasV.assign(model.outputBias.size(), 0.0);
        residualM.assign(model.contextResidualWeights.size(), 0.0);
        residualV.assign(model.contextResidualWeights.size(), 0.0);
    }
};

void applySgd(
    std::vector<double>& parameters,
    const std::vector<double>& gradients,
    double learningRate) noexcept
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

void applyAdam(
    std::vector<double>& parameters,
    std::vector<double>& firstMoment,
    std::vector<double>& secondMoment,
    const std::vector<double>& gradients,
    double learningRate,
    double beta1,
    double beta2,
    double epsilon,
    std::size_t step) noexcept
{
    const double firstCorrection =
        1.0 -
        std::pow(beta1,
                 static_cast<double>(step));

    const double secondCorrection =
        1.0 -
        std::pow(beta2,
                 static_cast<double>(step));

    for (std::size_t index = 0;
         index < parameters.size();
         ++index)
    {
        const auto gradient =
            gradients[index];

        firstMoment[index] =
            beta1 * firstMoment[index] +
            (1.0 - beta1) * gradient;

        secondMoment[index] =
            beta2 * secondMoment[index] +
            (1.0 - beta2) *
                gradient * gradient;

        const auto correctedFirst =
            firstMoment[index] /
            firstCorrection;

        const auto correctedSecond =
            secondMoment[index] /
            secondCorrection;

        parameters[index] -=
            learningRate *
            correctedFirst /
            (std::sqrt(correctedSecond) +
             epsilon);
    }
}

double gradientNorm(
    const std::vector<double>& input,
    const std::vector<double>& hiddenBias,
    const std::vector<double>& output,
    const std::vector<double>& outputBias,
    const std::vector<double>& residual) noexcept
{
    double squaredNorm = 0.0;

    for (const auto value : input)
        squaredNorm += value * value;
    for (const auto value : hiddenBias)
        squaredNorm += value * value;
    for (const auto value : output)
        squaredNorm += value * value;
    for (const auto value : outputBias)
        squaredNorm += value * value;

    for (const auto value : residual)
        squaredNorm += value * value;

    return std::sqrt(squaredNorm);
}

void scaleGradients(
    std::vector<double>& input,
    std::vector<double>& hiddenBias,
    std::vector<double>& output,
    std::vector<double>& outputBias,
    std::vector<double>& residual,
    double scale) noexcept
{
    for (auto& value : input)
        value *= scale;
    for (auto& value : hiddenBias)
        value *= scale;
    for (auto& value : output)
        value *= scale;
    for (auto& value : outputBias)
        value *= scale;
    for (auto& value : residual)
        value *= scale;
}

} // namespace

bool CompositionNeuralTrainingResult::isValid() const noexcept
{
    return trained &&
           epochsCompleted > 0 &&
           std::isfinite(initialLoss) &&
           std::isfinite(finalLoss) &&
           initialLoss >= 0.0 &&
           finalLoss >= 0.0;
}

CompositionNeuralTrainingResult
trainCompositionNeuralModel(
    CompositionNeuralModel& model,
    const CompositionDatasetPreparedView& prepared,
    const CompositionNeuralTrainingConfig& config) noexcept
{
    CompositionNeuralTrainingResult result;

    if (!model.isValid() ||
        !prepared.isValid() ||
        prepared.trainingCount() == 0 ||
        config.epochs == 0 ||
        !std::isfinite(config.learningRate) ||
        config.learningRate <= 0.0 ||
        !std::isfinite(config.gradientClip) ||
        config.gradientClip <= 0.0)
    {
        return result;
    }

    if (config.optimizer ==
            CompositionNeuralOptimizer::Adam &&
        (!std::isfinite(config.beta1) ||
         !std::isfinite(config.beta2) ||
         !std::isfinite(config.epsilon) ||
         config.beta1 <= 0.0 ||
         config.beta1 >= 1.0 ||
         config.beta2 <= 0.0 ||
         config.beta2 >= 1.0 ||
         config.epsilon <= 0.0))
    {
        return result;
    }

    const auto& batch =
        prepared.normalizedBatch;

    const auto inputWidth =
        model.contract.globalInputWidth +
        model.contract.sectionInputWidth;

    const auto hiddenWidth =
        CompositionNeuralModel::hiddenWidth;

    const auto outputWidth =
        model.contract.targetWidth;

    AdamState adam;
    adam.initialize(model);

    double previousLoss = 0.0;

    for (std::size_t epoch = 0;
         epoch < config.epochs;
         ++epoch)
    {
        double totalLoss = 0.0;
        std::size_t exampleCount = 0;

        for (const auto sampleIndex :
             prepared.partition.trainingIndices)
        {
            if (sampleIndex >= batch.sampleCount)
                return CompositionNeuralTrainingResult{};

            const auto sampleBase =
                sampleIndex *
                batch.maxSectionCount;

            const auto globalBase =
                sampleIndex *
                batch.globalFeatureWidth;

            for (std::size_t sectionIndex = 0;
                 sectionIndex + 1 <
                     batch.maxSectionCount;
                 ++sectionIndex)
            {
                if (batch.sectionMask[
                        sampleBase +
                        sectionIndex] == 0.0 ||
                    batch.sectionMask[
                        sampleBase +
                        sectionIndex +
                        1] == 0.0)
                {
                    continue;
                }

                const auto contextBase =
                    (sampleBase +
                     sectionIndex) *
                    batch.sectionFeatureWidth;

                const auto targetBase =
                    (sampleBase +
                     sectionIndex + 1) *
                    batch.sectionFeatureWidth;

                std::vector<double> inputs(
                    inputWidth,
                    0.0);

                for (std::size_t index = 0;
                     index <
                         batch.globalFeatureWidth;
                     ++index)
                {
                    inputs[index] =
                        batch.globalMatrix[
                            globalBase + index];
                }

                for (std::size_t index = 0;
                     index <
                         batch.sectionFeatureWidth;
                     ++index)
                {
                    inputs[
                        batch.globalFeatureWidth +
                        index] =
                        batch.sectionMatrix[
                            contextBase + index];
                }

                std::vector<double> hidden(
                    hiddenWidth,
                    0.0);

                for (std::size_t hiddenIndex = 0;
                     hiddenIndex < hiddenWidth;
                     ++hiddenIndex)
                {
                    double activation =
                        model.hiddenBias[hiddenIndex];

                    for (std::size_t inputIndex = 0;
                         inputIndex < inputWidth;
                         ++inputIndex)
                    {
                        activation +=
                            inputs[inputIndex] *
                            model.inputWeights[
                                inputIndex *
                                    hiddenWidth +
                                hiddenIndex];
                    }

                    hidden[hiddenIndex] =
                        std::tanh(activation);
                }

                std::vector<double> outputs(
                    outputWidth,
                    0.0);

                for (std::size_t outputIndex = 0;
                     outputIndex < outputWidth;
                     ++outputIndex)
                {
                    double activation =
                        model.outputBias[
                            outputIndex];

                    for (std::size_t hiddenIndex = 0;
                         hiddenIndex < hiddenWidth;
                         ++hiddenIndex)
                    {
                        activation +=
                            hidden[hiddenIndex] *
                            model.outputWeights[
                                hiddenIndex *
                                    outputWidth +
                                outputIndex];
                    }

                    outputs[outputIndex] =
                        std::tanh(activation);
                }

                std::vector<double> target(
                    outputWidth,
                    0.0);

                for (std::size_t featureIndex = 0;
                     featureIndex < outputWidth;
                     ++featureIndex)
                {
                    target[featureIndex] =
                        batch.sectionMatrix[
                            targetBase +
                            featureIndex];
                }

                totalLoss +=
                    squaredError(
                        outputs,
                        target);

                ++exampleCount;

                std::vector<double> outputGradient(
                    outputWidth,
                    0.0);

                for (std::size_t outputIndex = 0;
                     outputIndex < outputWidth;
                     ++outputIndex)
                {
                    const auto error =
                        outputs[outputIndex] -
                        target[outputIndex];

                    outputGradient[outputIndex] =
                        (2.0 /
                         static_cast<double>(
                             outputWidth)) *
                        error *
                        tanhDerivativeFromOutput(
                            outputs[outputIndex]);
                }

                std::vector<double> hiddenGradient(
                    hiddenWidth,
                    0.0);

                for (std::size_t hiddenIndex = 0;
                     hiddenIndex < hiddenWidth;
                     ++hiddenIndex)
                {
                    double gradient = 0.0;

                    for (std::size_t outputIndex = 0;
                         outputIndex < outputWidth;
                         ++outputIndex)
                    {
                        gradient +=
                            outputGradient[outputIndex] *
                            model.outputWeights[
                                hiddenIndex *
                                    outputWidth +
                                outputIndex];
                    }

                    hiddenGradient[hiddenIndex] =
                        gradient *
                        tanhDerivativeFromOutput(
                            hidden[hiddenIndex]);
                }

                std::vector<double> inputGradient(
                    model.inputWeights.size(),
                    0.0);

                std::vector<double> outputGradientWeights(
                    model.outputWeights.size(),
                    0.0);

                for (std::size_t hiddenIndex = 0;
                     hiddenIndex < hiddenWidth;
                     ++hiddenIndex)
                {
                    for (std::size_t outputIndex = 0;
                         outputIndex < outputWidth;
                         ++outputIndex)
                    {
                        outputGradientWeights[
                            hiddenIndex *
                                outputWidth +
                            outputIndex] =
                            hidden[hiddenIndex] *
                            outputGradient[outputIndex];
                    }
                }

                for (std::size_t inputIndex = 0;
                     inputIndex < inputWidth;
                     ++inputIndex)
                {
                    for (std::size_t hiddenIndex = 0;
                         hiddenIndex < hiddenWidth;
                         ++hiddenIndex)
                    {
                        inputGradient[
                            inputIndex *
                                hiddenWidth +
                            hiddenIndex] =
                            inputs[inputIndex] *
                            hiddenGradient[hiddenIndex];
                    }
                }

                std::vector<double> hiddenBiasGradient =
                    hiddenGradient;

                std::vector<double> outputBiasGradient =
                    outputGradient;

                std::vector<double> residualGradient(
                    model.contextResidualWeights.size(),
                    0.0);

                for (std::size_t contextIndex = 0;
                     contextIndex <
                         model.contract.sectionInputWidth;
                     ++contextIndex)
                {
                    for (std::size_t outputIndex = 0;
                         outputIndex < outputWidth;
                         ++outputIndex)
                    {
                        residualGradient[
                            contextIndex *
                                outputWidth +
                            outputIndex] =
                            inputs[
                                model.contract.globalInputWidth +
                                contextIndex] *
                            outputGradient[
                                outputIndex];
                    }
                }

                const auto norm =
                    gradientNorm(
                        inputGradient,
                        hiddenBiasGradient,
                        outputGradientWeights,
                        outputBiasGradient,
                        residualGradient);

                if (!std::isfinite(norm))
                    return CompositionNeuralTrainingResult{};

                if (norm >
                    config.gradientClip)
                {
                    const auto scale =
                        config.gradientClip /
                        norm;

                    scaleGradients(
                        inputGradient,
                        hiddenBiasGradient,
                        outputGradientWeights,
                        outputBiasGradient,
                        residualGradient,
                        scale);
                }

                if (config.optimizer ==
                    CompositionNeuralOptimizer::SGD)
                {
                    applySgd(
                        model.inputWeights,
                        inputGradient,
                        config.learningRate);

                    applySgd(
                        model.hiddenBias,
                        hiddenBiasGradient,
                        config.learningRate);

                    applySgd(
                        model.outputWeights,
                        outputGradientWeights,
                        config.learningRate);

                    applySgd(
                        model.outputBias,
                        outputBiasGradient,
                        config.learningRate);

                    applySgd(
                        model.contextResidualWeights,
                        residualGradient,
                        config.learningRate);
                }
                else
                {
                    ++adam.step;

                    applyAdam(
                        model.inputWeights,
                        adam.inputM,
                        adam.inputV,
                        inputGradient,
                        config.learningRate,
                        config.beta1,
                        config.beta2,
                        config.epsilon,
                        adam.step);

                    applyAdam(
                        model.hiddenBias,
                        adam.hiddenBiasM,
                        adam.hiddenBiasV,
                        hiddenBiasGradient,
                        config.learningRate,
                        config.beta1,
                        config.beta2,
                        config.epsilon,
                        adam.step);

                    applyAdam(
                        model.outputWeights,
                        adam.outputM,
                        adam.outputV,
                        outputGradientWeights,
                        config.learningRate,
                        config.beta1,
                        config.beta2,
                        config.epsilon,
                        adam.step);

                    applyAdam(
                        model.outputBias,
                        adam.outputBiasM,
                        adam.outputBiasV,
                        outputBiasGradient,
                        config.learningRate,
                        config.beta1,
                        config.beta2,
                        config.epsilon,
                        adam.step);

                    applyAdam(
                        model.contextResidualWeights,
                        adam.residualM,
                        adam.residualV,
                        residualGradient,
                        config.learningRate,
                        config.beta1,
                        config.beta2,
                        config.epsilon,
                        adam.step);
                }
            }
        }

        if (exampleCount == 0)
            return CompositionNeuralTrainingResult{};

        previousLoss =
            totalLoss /
            static_cast<double>(
                exampleCount);

        if (epoch == 0)
            result.initialLoss =
                previousLoss;

        result.epochsCompleted =
            epoch + 1;
    }

    result.finalLoss =
        previousLoss;
    result.trained = true;

    return result;
}

} // namespace midigengx::music
