#include "CompositionMidiSequenceWindow.h"

#include <algorithm>
#include <cmath>

namespace midigengx::music
{

bool CompositionMidiSequenceWindow::isValid(
    const CompositionSequenceLearningContract& contract)
    const noexcept
{
    if (!valid ||
        !contract.isValid() ||
        contextLength !=
            contract.contextLength ||
        featureWidth !=
            contract.inputFeatureWidth)
    {
        return false;
    }

    const auto expectedInput =
        contextLength *
        featureWidth;

    const auto expectedMask =
        contextLength;

    if (inputs.size() != expectedInput ||
        targets.size() !=
            featureWidth ||
        paddingMask.size() != expectedMask)
    {
        return false;
    }

    for (const auto value :
         inputs)
    {
        if (!std::isfinite(value) ||
            value < -1.0 ||
            value > 1.0)
            return false;
    }

    for (const auto value :
         targets)
    {
        if (!std::isfinite(value) ||
            value < -1.0 ||
            value > 1.0)
            return false;
    }

    for (const auto value :
         paddingMask)
    {
        if (!std::isfinite(value) ||
            value < 0.0 ||
            value > 1.0)
        {
            return false;
        }
    }

    return true;
}

CompositionMidiSequenceWindow
buildCompositionMidiSequenceWindow(
    const CompositionMidiTrainingSequence& sequence,
    const CompositionSequenceLearningContract& contract,
    const std::size_t targetIndex)
    noexcept
{
    CompositionMidiSequenceWindow window;

    if (!sequence.isValid() ||
        !contract.isValid() ||
        sequence.featureWidth !=
            contract.inputFeatureWidth ||
        targetIndex == 0 ||
        targetIndex >= sequence.events.size())
    {
        return window;
    }

    const auto width =
        contract.inputFeatureWidth;

    const auto contextLength =
        contract.contextLength;

    window.contextLength =
        contextLength;

    window.featureWidth =
        width;

    window.inputs.assign(
        contextLength * width,
        0.0);

    window.targets =
        sequence.events[
            targetIndex].features;

    window.paddingMask.assign(
        contextLength,
        0.0);

    const auto firstContextIndex =
        targetIndex > contextLength
            ? targetIndex - contextLength
            : 0;

    const auto available =
        targetIndex -
        firstContextIndex;

    const auto padding =
        contextLength -
        available;

    for (std::size_t contextIndex = 0;
         contextIndex < available;
         ++contextIndex)
    {
        const auto sourceIndex =
            firstContextIndex +
            contextIndex;

        const auto destinationIndex =
            padding +
            contextIndex;

        std::copy(
            sequence.events[
                sourceIndex].features.begin(),
            sequence.events[
                sourceIndex].features.end(),
            window.inputs.begin() +
                destinationIndex *
                width);

        window.paddingMask[
            destinationIndex] =
            1.0;
    }

    window.valid =
        true;

    if (!window.isValid(
            contract))
    {
        return {};
    }

    return window;
}

std::vector<CompositionMidiSequenceWindow>
buildCompositionMidiSequenceWindows(
    const CompositionMidiTrainingSequence& sequence,
    const CompositionSequenceLearningContract& contract)
    noexcept
{
    std::vector<CompositionMidiSequenceWindow> windows;

    if (!sequence.isValid() ||
        !contract.isValid() ||
        sequence.featureWidth !=
            contract.inputFeatureWidth ||
        sequence.events.size() < 2)
    {
        return windows;
    }

    const auto targetCount =
        sequence.events.size() - 1;

    windows.reserve(
        targetCount);

    for (std::size_t targetIndex = 1;
         targetIndex <= targetCount;
         ++targetIndex)
    {
        auto window =
            buildCompositionMidiSequenceWindow(
                sequence,
                contract,
                targetIndex);

        if (!window.isValid(
                contract))
        {
            return {};
        }

        windows.emplace_back(
            std::move(window));
    }

    return windows;
}

} // namespace midigengx::music
