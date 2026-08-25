#include "CompositionNeuralModel.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace midigengx::music
{
namespace
{

double deterministicWeight(
    std::size_t index) noexcept
{
    std::uint32_t state =
        static_cast<std::uint32_t>(
            0x9E3779B9u +
            static_cast<std::uint32_t>(
                index * 2654435761u));

    state ^= state >> 16;
    state *= 2246822519u;
    state ^= state >> 13;

    const auto normalized =
        static_cast<double>(
            state % 2001u) /
        1000.0 -
        1.0;

    return normalized * 0.05;
}

double tanhActivation(
    double value) noexcept
{
    return std::tanh(value);
}

std::size_t inputWidthFor(
    const CompositionLearningContract& contract) noexcept
{
    return contract.globalInputWidth +
           contract.sectionInputWidth;
}

} // namespace

bool CompositionNeuralPrediction::isValid(
    std::size_t expectedWidth) const noexcept
{
    if (!valid ||
        sectionFeatures.size() !=
            expectedWidth)
    {
        return false;
    }

    for (const auto value :
         sectionFeatures)
    {
        if (!std::isfinite(value))
            return false;
    }

    return true;
}

bool CompositionNeuralModel::isValid() const noexcept
{
    if (!initialized ||
        !contract.isValid() ||
        contract.objective !=
            LearningObjective::NextSectionPrediction)
    {
        return false;
    }

    const auto inputWidth =
        inputWidthFor(
            contract);

    return inputWeights.size() ==
               inputWidth *
               hiddenWidth &&
           hiddenBias.size() ==
               hiddenWidth &&
           outputWeights.size() ==
               hiddenWidth *
               contract.targetWidth &&
           outputBias.size() ==
               contract.targetWidth &&
           contextResidualWeights.size() ==
               contract.sectionInputWidth *
               contract.targetWidth;
}

CompositionNeuralModel
initializeCompositionNeuralModel(
    const CompositionLearningContract& contract) noexcept
{
    CompositionNeuralModel model;

    if (!contract.isValid() ||
        contract.objective !=
            LearningObjective::NextSectionPrediction)
    {
        return model;
    }

    const auto inputWidth =
        inputWidthFor(
            contract);

    model.contract =
        contract;

    model.inputWeights.resize(
        inputWidth *
        CompositionNeuralModel::hiddenWidth);

    model.hiddenBias.assign(
        CompositionNeuralModel::hiddenWidth,
        0.0);

    model.outputWeights.resize(
        CompositionNeuralModel::hiddenWidth *
        contract.targetWidth);

    model.outputBias.assign(
        contract.targetWidth,
        0.0);

    model.contextResidualWeights.assign(
        contract.sectionInputWidth *
            contract.targetWidth,
        0.0);

    for (std::size_t index = 0;
         index < model.inputWeights.size();
         ++index)
    {
        model.inputWeights[index] =
            deterministicWeight(
                index);
    }

    for (std::size_t index = 0;
         index < model.outputWeights.size();
         ++index)
    {
        model.outputWeights[index] =
            deterministicWeight(
                index +
                model.inputWeights.size());
    }

    model.initialized = true;
    return model;
}

CompositionNeuralPrediction
CompositionNeuralModel::predictNextSection(
    const std::vector<double>& globalFeatures,
    const std::vector<double>& contextSectionFeatures,
    bool contextIsValid) const noexcept
{
    CompositionNeuralPrediction prediction;

    if (!isValid() ||
        !contextIsValid ||
        globalFeatures.size() !=
            contract.globalInputWidth ||
        contextSectionFeatures.size() !=
            contract.sectionInputWidth)
    {
        return prediction;
    }

    const auto inputWidth =
        inputWidthFor(
            contract);

    std::vector<double> hidden(
        hiddenWidth,
        0.0);

    for (std::size_t hiddenIndex = 0;
         hiddenIndex < hiddenWidth;
         ++hiddenIndex)
    {
        double activation =
            hiddenBias[hiddenIndex];

        for (std::size_t inputIndex = 0;
             inputIndex < inputWidth;
             ++inputIndex)
        {
            const auto input =
                inputIndex <
                    contract.globalInputWidth
                    ? globalFeatures[inputIndex]
                    : contextSectionFeatures[
                        inputIndex -
                        contract.globalInputWidth];

            activation +=
                inputWeights[
                    inputIndex *
                        hiddenWidth +
                    hiddenIndex] *
                input;
        }

        hidden[hiddenIndex] =
            tanhActivation(
                activation);
    }

    prediction.sectionFeatures.assign(
        contract.targetWidth,
        0.0);

    for (std::size_t outputIndex = 0;
         outputIndex <
             contract.targetWidth;
         ++outputIndex)
    {
        double activation =
            outputBias[outputIndex];

        for (std::size_t hiddenIndex = 0;
             hiddenIndex < hiddenWidth;
             ++hiddenIndex)
        {
            activation +=
                hidden[
                    hiddenIndex] *
                outputWeights[
                    hiddenIndex *
                        contract.targetWidth +
                    outputIndex];
        }

        for (std::size_t contextIndex = 0;
             contextIndex <
                 contract.sectionInputWidth;
             ++contextIndex)
        {
            activation +=
                contextSectionFeatures[
                    contextIndex] *
                contextResidualWeights[
                    contextIndex *
                        contract.targetWidth +
                    outputIndex];
        }

        prediction.sectionFeatures[
            outputIndex] =
            std::clamp(
                std::tanh(
                    activation),
                -1.0,
                1.0);
    }

    prediction.valid = true;
    return prediction;
}

} // namespace midigengx::music
