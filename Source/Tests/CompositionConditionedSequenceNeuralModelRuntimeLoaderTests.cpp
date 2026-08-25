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

    expect(
        result.model.vocabulary.composers[0] ==
            "chopin",
        "embedded model preserves composer vocabulary");
}

void testRawResourceLoad()
{
    const auto artifact =
        serializeCompositionConditionedSequenceNeuralModel(
            makeModel());

    CompositionConditionedSequenceNeuralModelRuntimeLoader loader;

    const auto result =
        loader.load(
            artifact.bytes.data(),
            artifact.bytes.size());

    expect(
        result.isValid(),
        "embedded raw resource load succeeds");

    expect(
        result.model.vocabulary.styles[0] ==
            "romantic_piano",
        "embedded raw model preserves style vocabulary");
}

void testInvalidResourceFailsClosed()
{
    const std::vector<std::uint8_t> invalid =
    {
        0x00,
        0x01,
        0x02,
        0x03
    };

    CompositionConditionedSequenceNeuralModelRuntimeLoader loader;

    const auto result =
        loader.load(
            invalid);

    expect(
        !result.isValid(),
        "invalid embedded model fails closed");
}

void testNullResourceFailsClosed()
{
    CompositionConditionedSequenceNeuralModelRuntimeLoader loader;

    const auto result =
        loader.load(
            nullptr,
            0);

    expect(
        !result.isValid(),
        "null embedded resource fails closed");
}

} // namespace

int main()
{
    testEmbeddedVectorLoad();
    testRawResourceLoad();
    testInvalidResourceFailsClosed();
    testNullResourceFailsClosed();

    std::cout
        << "MIDI-GenGX Phase 112 conditioned model runtime loader tests passed.\n";

    return 0;
}
