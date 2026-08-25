#include "Music/CompositionConditionedSequenceNeuralEvaluator.h"

#include <cstdlib>
#include <iostream>
#include <string>

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

CompositionMidiTrainingSequence makeSequence(
    const std::string& id)
{
    CompositionMidiTrainingSequence sequence;
    sequence.sampleId = id;

    for (std::size_t index = 0;
         index < 80;
         ++index)
    {
        CompositionMidiTrainingEvent event;
        event.features =
        {
            0.0,
            60.0 / 127.0,
            0.5,
            0.75,
            0.0,
            0.25,
            0.5,
            0.25,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0
        };
        sequence.events.push_back(event);
    }

    sequence.featureWidth =
        CompositionMidiTrainingEvent::featureCount;

    sequence.analysisValid =
        true;
    return sequence;
}

CompositionSequenceMetadata makeMetadata(
    const std::string& id)
{
    CompositionSequenceMetadata metadata;
    metadata.sampleId = id;
    metadata.composerId = "chopin";
    metadata.workId = id + "_work";
    metadata.movementId = "single";
    metadata.styleId = "romantic";
    metadata.eraId = "romantic";
    metadata.instrumentationId = "solo_piano";
    metadata.verified = true;
    metadata.valid = true;
    return metadata;
}

void testEvaluatorProducesFiniteLoss()
{
    const auto metadata =
        buildCompositionSequenceMetadataCatalog(
        {
            makeMetadata("a"),
            makeMetadata("b")
        });

    const auto dataset =
        buildCompositionConditionedTrainingDataset(
        {
            makeSequence("a"),
            makeSequence("b")
        },
        metadata);

    expect(dataset.isValid(), "evaluation fixture dataset is valid");

    const auto contract =
        buildCompositionSequenceLearningContract(
            64,
            CompositionSequenceLearningObjective::NextEventPrediction);

    const auto model =
        initializeCompositionConditionedSequenceNeuralModel(
            contract,
            dataset.vocabulary);

    expect(model.isValid(), "evaluation fixture model is valid");

    const auto evaluation =
        evaluateCompositionConditionedSequenceNeuralModel(
            model,
            dataset);

    expect(evaluation.isValid(), "evaluation produces a valid result");
    expect(evaluation.sampleCount == 2 &&
           evaluation.windowCount > 0 &&
           evaluation.loss >= 0.0,
           "evaluation reports counts and non-negative loss");
}

void testInvalidDatasetFailsClosed()
{
    CompositionConditionedTrainingDataset invalid;

    CompositionConditioningVocabulary vocabulary;
    vocabulary.composers = {"chopin"};
    vocabulary.styles = {"romantic"};
    vocabulary.eras = {"romantic"};
    vocabulary.instrumentations = {"solo_piano"};
    vocabulary.valid = true;

    const auto contract =
        buildCompositionSequenceLearningContract(
            64,
            CompositionSequenceLearningObjective::NextEventPrediction);

    const auto model =
        initializeCompositionConditionedSequenceNeuralModel(
            contract,
            vocabulary);

    const auto evaluation =
        evaluateCompositionConditionedSequenceNeuralModel(
            model,
            invalid);

    expect(!evaluation.isValid(), "invalid dataset fails evaluation");
}

} // namespace

int main()
{
    testEvaluatorProducesFiniteLoss();
    testInvalidDatasetFailsClosed();

    std::cout
        << "MIDI-GenGX Phase 107 neural evaluator tests passed.\n";

    return 0;
}
