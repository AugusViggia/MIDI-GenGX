#pragma once

#include "CompositionMidiSequenceWindow.h"
#include "CompositionConditionedTrainingDataset.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace midigengx::music
{

struct CompositionConditionedSequenceNeuralPrediction
{
    std::vector<double> features;
    bool valid = false;

    bool isValid(
        std::size_t expectedWidth) const noexcept;
};

struct CompositionConditionedSequenceNeuralModel
{
    static constexpr int version = 1;
    static constexpr std::size_t hiddenWidth = 64;
    static constexpr std::size_t conditionEmbeddingWidth = 8;

    CompositionSequenceLearningContract contract;
    CompositionConditioningVocabulary vocabulary;

    std::vector<double> inputWeights;
    std::vector<double> recurrentWeights;
    std::vector<double> hiddenBias;
    std::vector<double> outputWeights;
    std::vector<double> outputBias;

    // One trainable embedding table per categorical condition. The four
    // embeddings are summed into the initial recurrent state.
    std::vector<double> composerEmbeddings;
    std::vector<double> styleEmbeddings;
    std::vector<double> eraEmbeddings;
    std::vector<double> instrumentationEmbeddings;

    bool initialized = false;

    bool isValid() const noexcept;

    bool predictNextEventInto(
        const CompositionMidiSequenceWindow& window,
        const CompositionConditionedTrainingSample& sample,
        CompositionConditionedSequenceNeuralPrediction& prediction) const noexcept;

    CompositionConditionedSequenceNeuralPrediction
    predictNextEvent(
        const CompositionMidiSequenceWindow& window,
        const CompositionConditionedTrainingSample& sample) const noexcept;
};

CompositionConditionedSequenceNeuralModel
initializeCompositionConditionedSequenceNeuralModel(
    const CompositionSequenceLearningContract& contract,
    const CompositionConditioningVocabulary& vocabulary) noexcept;

} // namespace midigengx::music
