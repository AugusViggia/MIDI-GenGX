#include "Music/CompositionSequenceNeuralTrainer.h"

#include <cstdlib>
#include <iostream>

using namespace midigengx::music;

namespace
{

void expect(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}

CompositionMidiTrainingSequence buildSequence(
    double offset)
{
    CompositionMidiTrainingSequence sequence;
    sequence.sampleId = "trainer";
    sequence.featureWidth =
        CompositionMidiTrainingEvent::featureCount;
    sequence.analysisValid = true;

    for (int index = 0; index < 80; ++index)
    {
        CompositionMidiTrainingEvent event;
        event.features.resize(
            CompositionMidiTrainingEvent::featureCount);

        for (std::size_t feature = 0;
             feature < event.features.size();
             ++feature)
        {
            event.features[feature] =
                0.1 *
                    static_cast<double>(
                        (feature + index) % 5) /
                    5.0 +
                offset;
        }

        sequence.events.push_back(event);
    }

    return sequence;
}

CompositionSequenceNeuralTrainingConfig config()
{
    CompositionSequenceNeuralTrainingConfig config;
    config.epochs = 20;
    config.learningRate = 0.001;
    config.gradientClip = 1.0;
    return config;
}

void testModelInitialization()
{
    const auto contract =
        buildCompositionSequenceLearningContract(
            16);

    const auto model =
        initializeCompositionSequenceNeuralModel(
            contract);

    expect(
        model.isValid(),
        "sequence model initializes with valid dimensions");
}

void testTrainingReducesLoss()
{
    const auto contract =
        buildCompositionSequenceLearningContract(
            16);

    auto model =
        initializeCompositionSequenceNeuralModel(
            contract);

    const std::vector<CompositionMidiTrainingSequence> sequences =
    {
        buildSequence(0.0),
        buildSequence(0.01)
    };

    const auto result =
        trainCompositionSequenceNeuralModel(
            model,
            sequences,
            config());

    expect(
        result.isValid(),
        "sequence neural training completes");

    expect(
        result.windowCount > 0,
        "sequence trainer builds training windows");

    expect(
        result.finalLoss <
            result.initialLoss,
        "sequence trainer reduces training loss");
}

void testPredictionIsValid()
{
    const auto contract =
        buildCompositionSequenceLearningContract(
            16);

    auto model =
        initializeCompositionSequenceNeuralModel(
            contract);

    const auto sequence =
        buildSequence(0.0);

    const auto windows =
        buildCompositionMidiSequenceWindows(
            sequence,
            contract);

    expect(
        !windows.empty(),
        "prediction test has windows");

    const auto prediction =
        model.predictNextEvent(
            windows.front());

    expect(
        prediction.isValid(
            contract.targetFeatureWidth),
        "untrained sequence model produces valid bounded prediction");
}

void testInvalidInputRejected()
{
    const auto contract =
        buildCompositionSequenceLearningContract(
            16);

    auto model =
        initializeCompositionSequenceNeuralModel(
            contract);

    CompositionSequenceNeuralTrainingConfig invalid;
    invalid.epochs = 0;

    const auto result =
        trainCompositionSequenceNeuralModel(
            model,
            {
                buildSequence(0.0)
            },
            invalid);

    expect(
        !result.trained,
        "invalid sequence training configuration is rejected");
}

void testTrainingIsDeterministic()
{
    const auto contract =
        buildCompositionSequenceLearningContract(
            16);

    auto firstModel =
        initializeCompositionSequenceNeuralModel(
            contract);

    auto secondModel =
        initializeCompositionSequenceNeuralModel(
            contract);

    const std::vector<CompositionMidiTrainingSequence> sequences =
    {
        buildSequence(0.0),
        buildSequence(0.02)
    };

    const auto first =
        trainCompositionSequenceNeuralModel(
            firstModel,
            sequences,
            config());

    const auto second =
        trainCompositionSequenceNeuralModel(
            secondModel,
            sequences,
            config());

    expect(
        first.isValid() &&
        second.isValid(),
        "deterministic sequence training runs are valid");

    expect(
        first.initialLoss ==
            second.initialLoss &&
        first.finalLoss ==
            second.finalLoss,
        "deterministic sequence training losses match");

    expect(
        firstModel.outputWeights ==
            secondModel.outputWeights,
        "deterministic sequence training weights match");
}

} // namespace

int main()
{
    testModelInitialization();
    testTrainingReducesLoss();
    testPredictionIsValid();
    testInvalidInputRejected();
    testTrainingIsDeterministic();

    std::cout
        << "MIDI-GenGX Phase 86 sequence neural trainer tests passed.\n";

    return 0;
}
