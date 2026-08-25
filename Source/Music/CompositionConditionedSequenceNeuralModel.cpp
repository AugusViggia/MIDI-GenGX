#include "CompositionConditionedSequenceNeuralModel.h"

#include <algorithm>
#include <cmath>

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

std::size_t embeddingSize(
    std::size_t categoryCount) noexcept
{
    return categoryCount *
        CompositionConditionedSequenceNeuralModel::conditionEmbeddingWidth;
}

void initializeEmbedding(
    std::vector<double>& embedding,
    std::size_t categoryCount,
    double scale)
{
    embedding.assign(
        embeddingSize(categoryCount),
        0.0);

    for (std::size_t category = 0;
         category < categoryCount;
         ++category)
    {
        for (std::size_t dimension = 0;
             dimension <
                 CompositionConditionedSequenceNeuralModel::conditionEmbeddingWidth;
             ++dimension)
        {
            const auto pattern =
                static_cast<int>(
                    (category * 17 +
                     dimension * 31) %
                    23) -
                11;

            embedding[
                category *
                    CompositionConditionedSequenceNeuralModel::conditionEmbeddingWidth +
                dimension] =
                static_cast<double>(
                    pattern) *
                scale;
        }
    }
}

void addEmbedding(
    std::vector<double>& hidden,
    const std::vector<double>& embedding,
    std::uint32_t categoryIndex)
{
    const auto width =
        CompositionConditionedSequenceNeuralModel::conditionEmbeddingWidth;

    const auto base =
        static_cast<std::size_t>(
            categoryIndex) *
        width;

    for (std::size_t index = 0;
         index < width && base + index < embedding.size();
         ++index)
    {
        hidden[index] +=
            embedding[base + index];
    }
}

} // namespace

bool CompositionConditionedSequenceNeuralPrediction::isValid(
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

bool CompositionConditionedSequenceNeuralModel::isValid()
    const noexcept
{
    if (!initialized ||
        !contract.isValid() ||
        !vocabulary.isValid() ||
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
            contract.targetFeatureWidth ||
        composerEmbeddings.size() !=
            embeddingSize(
                vocabulary.composers.size()) ||
        styleEmbeddings.size() !=
            embeddingSize(
                vocabulary.styles.size()) ||
        eraEmbeddings.size() !=
            embeddingSize(
                vocabulary.eras.size()) ||
        instrumentationEmbeddings.size() !=
            embeddingSize(
                vocabulary.instrumentations.size()))
    {
        return false;
    }

    return allFinite(inputWeights) &&
           allFinite(recurrentWeights) &&
           allFinite(hiddenBias) &&
           allFinite(outputWeights) &&
           allFinite(outputBias) &&
           allFinite(composerEmbeddings) &&
           allFinite(styleEmbeddings) &&
           allFinite(eraEmbeddings) &&
           allFinite(instrumentationEmbeddings);
}

CompositionConditionedSequenceNeuralModel
initializeCompositionConditionedSequenceNeuralModel(
    const CompositionSequenceLearningContract& contract,
    const CompositionConditioningVocabulary& vocabulary)
    noexcept
{
    CompositionConditionedSequenceNeuralModel model;

    if (!contract.isValid() ||
        !vocabulary.isValid())
    {
        return model;
    }

    model.contract =
        contract;

    model.vocabulary =
        vocabulary;

    model.inputWeights.assign(
        CompositionConditionedSequenceNeuralModel::hiddenWidth *
            contract.inputFeatureWidth,
        0.0);

    model.recurrentWeights.assign(
        CompositionConditionedSequenceNeuralModel::hiddenWidth *
            CompositionConditionedSequenceNeuralModel::hiddenWidth,
        0.0);

    model.hiddenBias.assign(
        CompositionConditionedSequenceNeuralModel::hiddenWidth,
        0.0);

    model.outputWeights.assign(
        contract.targetFeatureWidth *
            CompositionConditionedSequenceNeuralModel::hiddenWidth,
        0.0);

    model.outputBias.assign(
        contract.targetFeatureWidth,
        0.0);

    const auto inputScale =
        0.05 /
        std::sqrt(
            static_cast<double>(
                contract.inputFeatureWidth));

    for (std::size_t row = 0;
         row < CompositionConditionedSequenceNeuralModel::hiddenWidth;
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
                row * contract.inputFeatureWidth +
                column] =
                static_cast<double>(
                    pattern) *
                inputScale;
        }
    }

    const auto recurrentScale =
        0.05 /
        std::sqrt(
            static_cast<double>(
                CompositionConditionedSequenceNeuralModel::hiddenWidth));

    for (std::size_t row = 0;
         row < CompositionConditionedSequenceNeuralModel::hiddenWidth;
         ++row)
    {
        for (std::size_t column = 0;
             column < CompositionConditionedSequenceNeuralModel::hiddenWidth;
             ++column)
        {
            if (row == column)
            {
                model.recurrentWeights[
                    row *
                        CompositionConditionedSequenceNeuralModel::hiddenWidth +
                    column] =
                    recurrentScale;
            }
        }
    }

    const auto outputScale =
        0.05 /
        std::sqrt(
            static_cast<double>(
                CompositionConditionedSequenceNeuralModel::hiddenWidth));

    for (std::size_t row = 0;
         row < contract.targetFeatureWidth;
         ++row)
    {
        for (std::size_t column = 0;
             column < CompositionConditionedSequenceNeuralModel::hiddenWidth;
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
                    CompositionConditionedSequenceNeuralModel::hiddenWidth +
                column] =
                static_cast<double>(
                    pattern) *
                outputScale;
        }
    }

    const auto embeddingScale =
        0.025;

    initializeEmbedding(
        model.composerEmbeddings,
        vocabulary.composers.size(),
        embeddingScale);

    initializeEmbedding(
        model.styleEmbeddings,
        vocabulary.styles.size(),
        embeddingScale);

    initializeEmbedding(
        model.eraEmbeddings,
        vocabulary.eras.size(),
        embeddingScale);

    initializeEmbedding(
        model.instrumentationEmbeddings,
        vocabulary.instrumentations.size(),
        embeddingScale);

    model.initialized =
        true;

    return model;
}

CompositionConditionedSequenceNeuralPrediction
CompositionConditionedSequenceNeuralModel::predictNextEvent(
    const CompositionMidiSequenceWindow& window,
    const CompositionConditionedTrainingSample& sample)
    const noexcept
{
    CompositionConditionedSequenceNeuralPrediction prediction;

    if (!isValid() ||
        !window.isValid(contract) ||
        !sample.isValid() ||
        sample.composerIndex >=
            vocabulary.composers.size() ||
        sample.styleIndex >=
            vocabulary.styles.size() ||
        sample.eraIndex >=
            vocabulary.eras.size() ||
        sample.instrumentationIndex >=
            vocabulary.instrumentations.size())
    {
        return prediction;
    }

    const auto width =
        contract.inputFeatureWidth;

    std::vector<double> hidden(
        hiddenWidth,
        0.0);

    addEmbedding(
        hidden,
        composerEmbeddings,
        sample.composerIndex);

    addEmbedding(
        hidden,
        styleEmbeddings,
        sample.styleIndex);

    addEmbedding(
        hidden,
        eraEmbeddings,
        sample.eraIndex);

    addEmbedding(
        hidden,
        instrumentationEmbeddings,
        sample.instrumentationIndex);

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
            time *
            width;

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
                row *
                hiddenWidth;

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
                std::tanh(sum);
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
            row *
            hiddenWidth;

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
