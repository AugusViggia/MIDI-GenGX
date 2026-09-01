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

void addConditionEmbedding(
    std::vector<double>& hidden,
    const std::vector<double>& embedding,
    std::uint32_t categoryIndex) noexcept
{
    constexpr auto width =
        CompositionConditionedSequenceNeuralModel::conditionEmbeddingWidth;

    const auto base =
        static_cast<std::size_t>(
            categoryIndex) *
        width;

    for (std::size_t index = 0;
         index < width;
         ++index)
    {
        hidden[index] +=
            embedding[base + index];
    }
}

void addInitialConditionEmbeddings(
    std::vector<double>& hidden,
    const std::vector<double>& composerEmbeddings,
    std::uint32_t composerIndex,
    const std::vector<double>& styleEmbeddings,
    std::uint32_t styleIndex,
    const std::vector<double>& eraEmbeddings,
    std::uint32_t eraIndex,
    const std::vector<double>& instrumentationEmbeddings,
    std::uint32_t instrumentationIndex) noexcept
{
    constexpr auto width =
        CompositionConditionedSequenceNeuralModel::conditionEmbeddingWidth;

    const auto composerBase =
        static_cast<std::size_t>(composerIndex) * width;
    const auto styleBase =
        static_cast<std::size_t>(styleIndex) * width;
    const auto eraBase =
        static_cast<std::size_t>(eraIndex) * width;
    const auto instrumentationBase =
        static_cast<std::size_t>(instrumentationIndex) * width;

    for (std::size_t index = 0;
         index < width;
         ++index)
    {
        hidden[index] +=
            composerEmbeddings[composerBase + index] +
            styleEmbeddings[styleBase + index] +
            eraEmbeddings[eraBase + index] +
            instrumentationEmbeddings[instrumentationBase + index];
    }
}

bool hasValidInferenceStructure(
    const CompositionConditionedSequenceNeuralModel& model,
    const CompositionMidiSequenceWindow& window,
    const CompositionConditionedTrainingSample& sample) noexcept
{
    return model.initialized &&
           window.valid &&
           window.contextLength ==
               model.contract.contextLength &&
           window.featureWidth ==
               model.contract.inputFeatureWidth &&
           window.inputs.size() ==
               model.contract.contextLength *
               model.contract.inputFeatureWidth &&
           window.targets.size() ==
               model.contract.targetFeatureWidth &&
           window.paddingMask.size() ==
               model.contract.contextLength &&
           sample.valid &&
           sample.composerIndex <
               model.vocabulary.composers.size() &&
           sample.styleIndex <
               model.vocabulary.styles.size() &&
           sample.eraIndex <
               model.vocabulary.eras.size() &&
           sample.instrumentationIndex <
               model.vocabulary.instrumentations.size();
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

bool CompositionConditionedSequenceNeuralModel::predictNextEventInto(
    const CompositionMidiSequenceWindow& window,
    const CompositionConditionedTrainingSample& sample,
    CompositionConditionedSequenceNeuralPrediction& prediction)
    const noexcept
{
    prediction.valid = false;

    if (!hasValidInferenceStructure(
            *this,
            window,
            sample))
    {
        prediction.features.clear();
        return false;
    }

    const auto width =
        contract.inputFeatureWidth;

    thread_local std::vector<double> hidden;
    thread_local std::vector<double> nextHidden;

    hidden.resize(hiddenWidth);
    nextHidden.resize(hiddenWidth);

    std::fill(
        hidden.begin(),
        hidden.end(),
        0.0);

    addInitialConditionEmbeddings(
        hidden,
        composerEmbeddings,
        sample.composerIndex,
        styleEmbeddings,
        sample.styleIndex,
        eraEmbeddings,
        sample.eraIndex,
        instrumentationEmbeddings,
        sample.instrumentationIndex);

    for (std::size_t time = 0;
         time < contract.contextLength;
         ++time)
    {
        if (window.paddingMask[time] <= 0.0)
            continue;

        std::fill(
            nextHidden.begin(),
            nextHidden.end(),
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

            const auto inputWeightBase =
                row * width;

            const auto* inputWeight =
                inputWeights.data() +
                inputWeightBase;

            const auto* inputValue =
                window.inputs.data() +
                inputBase;

            for (std::size_t column = 0;
                 column < width;
                 ++column)
            {
                sum +=
                    inputWeight[column] *
                    inputValue[column];
            }

            const auto recurrentBase =
                row *
                hiddenWidth;

            const auto* recurrentWeight =
                recurrentWeights.data() +
                recurrentBase;

            for (std::size_t column = 0;
                 column < hiddenWidth;
                 ++column)
            {
                sum +=
                    recurrentWeight[column] *
                    hidden[column];
            }

            nextHidden[row] =
                std::tanh(sum);
        }

        hidden.swap(
            nextHidden);
    }

    const auto targetWidth =
        contract.targetFeatureWidth;

    prediction.features.resize(
        targetWidth);

    for (std::size_t row = 0;
         row < targetWidth;
         ++row)
    {
        double sum =
            outputBias[row];

        const auto outputBase =
            row *
            hiddenWidth;

        const auto* outputWeight =
            outputWeights.data() +
            outputBase;

        for (std::size_t column = 0;
             column < hiddenWidth;
             ++column)
        {
            sum +=
                outputWeight[column] *
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

    return true;
}

CompositionConditionedSequenceNeuralPrediction
CompositionConditionedSequenceNeuralModel::predictNextEvent(
    const CompositionMidiSequenceWindow& window,
    const CompositionConditionedTrainingSample& sample)
    const noexcept
{
    CompositionConditionedSequenceNeuralPrediction prediction;

    predictNextEventInto(
        window,
        sample,
        prediction);

    return prediction;
}

} // namespace midigengx::music
