#include "Music/CompositionConditionedSequenceNeuralModelRuntimeLoader.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

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

CompositionConditionedSequenceNeuralModel makeModel()
{
    const auto contract =
        buildCompositionSequenceLearningContract(
            16,
            CompositionSequenceLearningObjective::NextEventPrediction);

    CompositionConditioningVocabulary vocabulary;

    vocabulary.composers =
    {
        "chopin"
    };

    vocabulary.styles =
    {
        "romantic_piano"
    };

    vocabulary.eras =
    {
        "romantic"
    };

    vocabulary.instrumentations =
    {
        "solo_piano"
    };

    vocabulary.valid =
        true;

    return initializeCompositionConditionedSequenceNeuralModel(
        contract,
        vocabulary);
}

void testEmbeddedVectorLoad()
{
    const auto artifact =
        serializeCompositionConditionedSequenceNeuralModel(
            makeModel());

    CompositionConditionedSequenceNeuralModelRuntimeLoader loader;

    const auto result =
        loader.load(
            artifact.bytes);

    expect(
        result.isValid(),
        "embedded vector model load succeeds");
}

void testOversizedRawResourceFailsBeforeAllocation()
{
    const std::uint8_t sentinel = 0;

    CompositionConditionedSequenceNeuralModelRuntimeLoader loader;

    const auto result =
        loader.load(
            &sentinel,
            CompositionConditionedSequenceNeuralModelRuntimeLoader::maxArtifactBytes + 1);

    expect(
        !result.isValid(),
        "oversized embedded model is rejected before allocation");
}

} // namespace

int main()
{
    testEmbeddedVectorLoad();
    testOversizedRawResourceFailsBeforeAllocation();

    std::cout
        << "MIDI-GenGX Phase 163 conditioned runtime loader robustness tests passed.\n";

    return 0;
}
