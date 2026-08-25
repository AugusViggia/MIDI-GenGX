#include "Music/CompositionConditionedSequenceNeuralTrainer.h"

#include <algorithm>
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

CompositionMidiTrainingSequence makeSequence(
    const std::string& id,
    double offset)
{
    CompositionMidiTrainingSequence sequence;
    sequence.sampleId = id;
    sequence.analysisValid = true;

    for (int index = 0;
         index < 80;
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
                std::clamp(
                    0.1 *
                        static_cast<double>(
                            (feature + index) % 5) /
                        5.0 +
                    offset,
                    -1.0,
                    1.0);
        }

        sequence.events.push_back(
            event);
    }

    return sequence;
}

CompositionSequenceMetadata makeMetadata(
    const std::string& id,
    const std::string& composer,
    const std::string& style,
    const std::string& era,
    const std::string& instrumentation)
{
    CompositionSequenceMetadata metadata;

    metadata.sampleId = id;
    metadata.composerId = composer;
    metadata.workId = id + "_work";
    metadata.movementId = "movement_1";
    metadata.styleId = style;
    metadata.eraId = era;
    metadata.instrumentationId =
        instrumentation;
    metadata.verified = true;
    metadata.valid = true;

    return metadata;
}

CompositionConditionedTrainingDataset buildDataset()
{
    const auto metadataCatalog =
        buildCompositionSequenceMetadataCatalog(
        {
            makeMetadata(
                "piece-a",
                "chopin",
                "romantic_piano",
                "romantic",
                "solo_piano"),
            makeMetadata(
                "piece-b",
                "bach",
                "baroque_counterpoint",
                "baroque",
                "solo_piano"),
            makeMetadata(
                "piece-c",
                "chopin",
                "romantic_piano",
                "romantic",
                "solo_piano")
        });

    return buildCompositionConditionedTrainingDataset(
        {
            makeSequence(
                "piece-a",
                0.0),
            makeSequence(
                "piece-b",
                0.02),
            makeSequence(
                "piece-c",
                0.01)
        },
        metadataCatalog);
}

void testModelInitialization()
{
    const auto dataset =
        buildDataset();

    const auto contract =
        buildCompositionSequenceLearningContract(
            16);

    auto model =
        initializeCompositionConditionedSequenceNeuralModel(
            contract,
            dataset.vocabulary);

    expect(
        dataset.isValid(),
        "conditioned fixture dataset is valid");

    expect(
        model.isValid(),
        "conditioned neural model initializes");
}

void testPredictionUsesCondition()
{
    const auto dataset =
        buildDataset();

    const auto contract =
        buildCompositionSequenceLearningContract(
            16);

    const auto model =
        initializeCompositionConditionedSequenceNeuralModel(
            contract,
            dataset.vocabulary);

    const auto& first =
        dataset.samples[0];

    const auto& second =
        dataset.samples[1];

    const auto firstWindows =
        buildCompositionMidiSequenceWindows(
            first.sequence,
            contract);

    const auto secondWindows =
        buildCompositionMidiSequenceWindows(
            second.sequence,
            contract);

    expect(
        !firstWindows.empty() &&
        !secondWindows.empty(),
        "conditioned prediction fixture has windows");

    const auto firstPrediction =
        model.predictNextEvent(
            firstWindows.front(),
            first);

    const auto secondPrediction =
        model.predictNextEvent(
            firstWindows.front(),
            second);

    expect(
        firstPrediction.isValid(
            contract.targetFeatureWidth) &&
        secondPrediction.isValid(
            contract.targetFeatureWidth),
        "conditioned predictions are valid");

    expect(
        firstPrediction.features !=
            secondPrediction.features,
        "different style/composer conditions affect prediction");
}

void testTrainingReducesLoss()
{
    const auto dataset =
        buildDataset();

    const auto contract =
        buildCompositionSequenceLearningContract(
            16);

    auto model =
        initializeCompositionConditionedSequenceNeuralModel(
            contract,
            dataset.vocabulary);

    CompositionConditionedSequenceNeuralTrainingConfig config;
    config.epochs = 16;
    config.learningRate = 0.001;
    config.gradientClip = 1.0;

    const auto result =
        trainCompositionConditionedSequenceNeuralModel(
            model,
            dataset,
            config);

    expect(
        result.isValid(),
        "conditioned training completes");

    expect(
        result.windowCount > 0,
        "conditioned trainer creates sequence windows");

    expect(
        result.finalLoss <
            result.initialLoss,
        "conditioned trainer reduces loss");
}

void testTrainingIsDeterministic()
{
    const auto dataset =
        buildDataset();

    const auto contract =
        buildCompositionSequenceLearningContract(
            16);

    auto firstModel =
        initializeCompositionConditionedSequenceNeuralModel(
            contract,
            dataset.vocabulary);

    auto secondModel =
        initializeCompositionConditionedSequenceNeuralModel(
            contract,
            dataset.vocabulary);

    CompositionConditionedSequenceNeuralTrainingConfig config;
    config.epochs = 8;
    config.learningRate = 0.001;
    config.gradientClip = 1.0;

    const auto first =
        trainCompositionConditionedSequenceNeuralModel(
            firstModel,
            dataset,
            config);

    const auto second =
        trainCompositionConditionedSequenceNeuralModel(
            secondModel,
            dataset,
            config);

    expect(
        first.isValid() &&
        second.isValid(),
        "deterministic conditioned training completes");

    expect(
        first.finalLoss ==
            second.finalLoss,
        "deterministic conditioned training losses match");

    expect(
        firstModel.composerEmbeddings ==
            secondModel.composerEmbeddings,
        "condition embeddings train deterministically");
}

} // namespace

int main()
{
    testModelInitialization();
    testPredictionUsesCondition();
    testTrainingReducesLoss();
    testTrainingIsDeterministic();

    std::cout
        << "MIDI-GenGX Phase 91 conditioned sequence neural trainer tests passed.\n";

    return 0;
}
