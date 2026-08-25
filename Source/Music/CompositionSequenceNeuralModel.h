#pragma once

#include "CompositionMidiSequenceWindow.h"

#include <cstddef>
#include <vector>

namespace midigengx::music
{

struct CompositionSequenceNeuralPrediction
{
    std::vector<double> features;
    bool valid = false;

    bool isValid(
        std::size_t expectedWidth) const noexcept;
};

struct CompositionSequenceNeuralModel
{
    static constexpr int version = 1;
    static constexpr std::size_t hiddenWidth = 64;

    CompositionSequenceLearningContract contract;

    // Simple recurrent sequence model:
    // hidden_t = tanh(Wx*x_t + Wh*hidden_(t-1) + b_h)
    // output   = W_out*hidden_T + b_out
    std::vector<double> inputWeights;
    std::vector<double> recurrentWeights;
    std::vector<double> hiddenBias;
    std::vector<double> outputWeights;
    std::vector<double> outputBias;

    bool initialized = false;

    bool isValid() const noexcept;

    CompositionSequenceNeuralPrediction
    predictNextEvent(
        const CompositionMidiSequenceWindow& window) const noexcept;
};

CompositionSequenceNeuralModel
initializeCompositionSequenceNeuralModel(
    const CompositionSequenceLearningContract& contract) noexcept;

} // namespace midigengx::music
