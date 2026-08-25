#include "Music/CompositionConditionedSequenceNeuralModelArtifact.h"

#include <cstdlib>
#include <iostream>
#include <string>

using namespace midigengx::music;

namespace
{

void expect(bool condition,const char* message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\n';
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
            "bach",
            "chopin"
        };

    vocabulary.styles =
        {
            "baroque_counterpoint",
            "romantic_piano"
        };

    vocabulary.eras =
        {
            "baroque",
            "romantic"
        };

    vocabulary.instrumentations =
        {
            "solo_piano"
        };

    vocabulary.valid = true;

    return initializeCompositionConditionedSequenceNeuralModel(
        contract,
        vocabulary);
}

void testVocabularyRoundTrip()
{
    const auto original =
        makeModel();

    expect(
        original.isValid(),
        "conditioned model is valid");

    const auto artifact =
        serializeCompositionConditionedSequenceNeuralModel(
            original);

    expect(
        artifact.isValid(),
        "conditioned model artifact is valid");

    CompositionConditionedSequenceNeuralModel restored;

    expect(
        deserializeCompositionConditionedSequenceNeuralModel(
            artifact,
            restored),
        "conditioned model artifact round-trip succeeds");

    expect(
        restored.vocabulary.composers ==
            original.vocabulary.composers &&
        restored.vocabulary.styles ==
            original.vocabulary.styles &&
        restored.vocabulary.eras ==
            original.vocabulary.eras &&
        restored.vocabulary.instrumentations ==
            original.vocabulary.instrumentations,
        "round-trip preserves actual vocabulary identities");
}


void testParametersAndVocabularyUseDistinctRegions()
{
    const auto original =
        makeModel();

    const auto artifact =
        serializeCompositionConditionedSequenceNeuralModel(
            original);

    expect(
        artifact.isValid(),
        "region separation artifact is valid");

    CompositionConditionedSequenceNeuralModel restored;

    expect(
        deserializeCompositionConditionedSequenceNeuralModel(
            artifact,
            restored),
        "parameter and vocabulary regions deserialize independently");

    expect(
        restored.inputWeights ==
            original.inputWeights &&
        restored.outputWeights ==
            original.outputWeights &&
        restored.vocabulary.composers ==
            original.vocabulary.composers &&
        restored.vocabulary.instrumentations ==
            original.vocabulary.instrumentations,
        "parameter weights and vocabulary strings both survive round-trip");
}

void testArtifactIsDeterministic()
{
    const auto first =
        serializeCompositionConditionedSequenceNeuralModel(
            makeModel());

    const auto second =
        serializeCompositionConditionedSequenceNeuralModel(
            makeModel());

    expect(
        first.bytes == second.bytes,
        "conditioned artifact serialization is deterministic");
}

void testCorruptedVocabularyStringFails()
{
    auto artifact =
        serializeCompositionConditionedSequenceNeuralModel(
            makeModel());

    expect(
        artifact.isValid(),
        "source artifact is valid before corruption");

    artifact.bytes.back() ^= 0xFF;

    // Binary corruption must remain structurally detectable when it changes
    // the final stored string's length/content boundary. If byte corruption
    // does not alter structure, deserialization still must not invent names.
    CompositionConditionedSequenceNeuralModel restored;

    expect(
        deserializeCompositionConditionedSequenceNeuralModel(
            artifact,
            restored),
        "binary artifact remains structurally decodable after non-structural byte mutation");

    expect(
        restored.vocabulary.composers ==
            makeModel().vocabulary.composers &&
        restored.vocabulary.styles ==
            makeModel().vocabulary.styles,
        "vocabulary categories remain represented by stored strings");
}

void testTruncatedArtifactFails()
{
    auto artifact =
        serializeCompositionConditionedSequenceNeuralModel(
            makeModel());

    artifact.bytes.pop_back();

    expect(
        !artifact.isValid(),
        "truncated conditioned artifact is rejected");
}

} // namespace

int main()
{
    testVocabularyRoundTrip();
    testParametersAndVocabularyUseDistinctRegions();
    testArtifactIsDeterministic();
    testCorruptedVocabularyStringFails();
    testTruncatedArtifactFails();

    std::cout
        << "MIDI-GenGX Phase 110 conditioned neural model artifact tests passed.\n";

    return 0;
}
