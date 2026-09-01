#pragma once

#include "CompositionMidiTrainingSequence.h"
#include "CompositionSequenceLearningContract.h"

#include <cstddef>
#include <vector>

namespace midigengx::music
{

struct CompositionMidiSequenceWindow
{
    std::vector<double> inputs;
    std::vector<double> targets;
    std::vector<double> paddingMask;

    std::size_t contextLength = 0;
    std::size_t featureWidth = 0;

    bool valid = false;

    bool isValid(
        const CompositionSequenceLearningContract& contract) const noexcept;
};

CompositionMidiSequenceWindow
buildCompositionMidiSequenceWindow(
    const CompositionMidiTrainingSequence& sequence,
    const CompositionSequenceLearningContract& contract,
    std::size_t targetIndex) noexcept;

std::vector<CompositionMidiSequenceWindow>
buildCompositionMidiSequenceWindows(
    const CompositionMidiTrainingSequence& sequence,
    const CompositionSequenceLearningContract& contract) noexcept;

} // namespace midigengx::music
