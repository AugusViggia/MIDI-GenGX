#include "Music/CompositionMidiSequenceWindow.h"

#include <cstdlib>
#include <iostream>

using namespace midigengx::music;

namespace
{

void expect(
    bool condition,
    const char* message)
{
    if (!condition)
    {
        std::cerr
            << "FAILED: "
            << message
            << '\n';
        std::exit(1);
    }
}

CompositionMidiTrainingSequence buildSequence()
{
    CompositionMidiTrainingSequence sequence;

    sequence.sampleId =
        "phase85";

    sequence.featureWidth =
        CompositionMidiTrainingEvent::featureCount;

    sequence.analysisValid =
        true;

    for (int index = 0;
         index < 10;
         ++index)
    {
        CompositionMidiTrainingEvent event;

        event.features.resize(
            CompositionMidiTrainingEvent::featureCount);

        for (std::size_t feature = 0;
             feature < event.features.size();
             ++feature)
        {
            event.features[feature] =
                static_cast<double>(
                    (index + 1) *
                    (feature + 1)) /
                500.0;
        }

        sequence.events.push_back(
            event);
    }

    return sequence;
}

void testContractIsValid()
{
    const auto contract =
        buildCompositionSequenceLearningContract(
            4);

    expect(
        contract.isValid(),
        "sequence learning contract is valid");

    expect(
        contract.inputFeatureWidth ==
            CompositionMidiTrainingEvent::featureCount,
        "contract input width matches event schema");

    expect(
        contract.targetFeatureWidth ==
            CompositionMidiTrainingEvent::featureCount,
        "contract target width matches event schema");
}

void testWindowCount()
{
    const auto sequence =
        buildSequence();

    const auto contract =
        buildCompositionSequenceLearningContract(
            4);

    expect(
        sequence.isValid(),
        "window-count fixture sequence is valid");

    expect(
        contract.isValid(),
        "window-count fixture contract is valid");

    const auto windows =
        buildCompositionMidiSequenceWindows(
            sequence,
            contract);

    expect(
        windows.size() ==
            sequence.events.size() - 1,
        "next-event windows equal sequence length minus one");
}

void testWindowShape()
{
    const auto sequence =
        buildSequence();

    const auto contract =
        buildCompositionSequenceLearningContract(
            4);

    const auto windows =
        buildCompositionMidiSequenceWindows(
            sequence,
            contract);

    expect(
        !windows.empty(),
        "window builder produces training examples");

    for (const auto& window :
         windows)
    {
        expect(
            window.isValid(
                contract),
            "every sequence window is valid");

        expect(
            window.inputs.size() ==
                contract.contextLength *
                contract.inputFeatureWidth,
            "window input shape matches contract");

        expect(
            window.targets.size() ==
                contract.targetFeatureWidth,
            "window target shape matches contract");

        expect(
            window.paddingMask.size() ==
                contract.contextLength,
            "window mask shape matches context length");
    }
}

void testInitialPaddingIsExplicit()
{
    const auto sequence =
        buildSequence();

    const auto contract =
        buildCompositionSequenceLearningContract(
            4);

    const auto windows =
        buildCompositionMidiSequenceWindows(
            sequence,
            contract);

    expect(
        windows.front().paddingMask[0] == 0.0 &&
        windows.front().paddingMask[1] == 0.0 &&
        windows.front().paddingMask[2] == 0.0 &&
        windows.front().paddingMask[3] == 1.0,
        "first window explicitly marks padded context positions");

    expect(
        windows[4].paddingMask[0] == 1.0,
        "later windows retain non-padded context");
}

void testDeterministicWindowing()
{
    const auto sequence =
        buildSequence();

    const auto contract =
        buildCompositionSequenceLearningContract(
            5);

    const auto first =
        buildCompositionMidiSequenceWindows(
            sequence,
            contract);

    const auto second =
        buildCompositionMidiSequenceWindows(
            sequence,
            contract);

    expect(
        first.size() == second.size(),
        "windowing is deterministic");

    for (std::size_t index = 0;
         index < first.size();
         ++index)
    {
        expect(
            first[index].inputs ==
                second[index].inputs &&
            first[index].targets ==
                second[index].targets &&
            first[index].paddingMask ==
                second[index].paddingMask,
            "deterministic sequence windows are identical");
    }
}

void testInvalidContractRejected()
{
    const auto sequence =
        buildSequence();

    const auto invalid =
        buildCompositionSequenceLearningContract(
            2);

    const auto windows =
        buildCompositionMidiSequenceWindows(
            sequence,
            invalid);

    expect(
        windows.empty(),
        "invalid sequence contract is rejected");
}

} // namespace

int main()
{
    testContractIsValid();
    testWindowCount();
    testWindowShape();
    testInitialPaddingIsExplicit();
    testDeterministicWindowing();
    testInvalidContractRejected();

    std::cout
        << "MIDI-GenGX Phase 85 sequence window tests passed.\n";

    return 0;
}
