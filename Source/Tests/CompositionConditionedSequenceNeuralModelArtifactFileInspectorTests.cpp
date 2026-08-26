#include "Music/CompositionConditionedSequenceNeuralModelArtifactFileInspector.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace midigengx::music;

namespace
{

void expect(
    const bool condition,
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

CompositionConditionedSequenceNeuralModel
makeModel()
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

void testArtifactFileRoundTrip()
{
    const auto artifact =
        serializeCompositionConditionedSequenceNeuralModel(
            makeModel());

    expect(
        artifact.isValid(),
        "fixture artifact is valid");

    const auto path =
        std::filesystem::temp_directory_path() /
        "MIDI_GenGX_Phase113_ArtifactInspection.mgcn";

    {
        std::ofstream file(
            path,
            std::ios::binary);

        expect(
            static_cast<bool>(file),
            "temporary artifact file can be created");

        file.write(
            reinterpret_cast<const char*>(
                artifact.bytes.data()),
            static_cast<std::streamsize>(
                artifact.bytes.size()));
    }

    const auto result =
        inspectCompositionConditionedSequenceNeuralModelArtifactFile(
            path.string());

    expect(
        result.isValid(),
        "artifact file inspection succeeds");

    expect(
        result.artifactVersion ==
            CompositionConditionedSequenceNeuralModelArtifact::version,
        "artifact version matches current runtime format");

    expect(
        result.composerSummary == "chopin" &&
        result.styleSummary == "romantic_piano" &&
        result.eraSummary == "romantic" &&
        result.instrumentationSummary == "solo_piano",
        "artifact inspection preserves conditioning vocabulary");

    std::error_code error;
    std::filesystem::remove(
        path,
        error);
}

void testMissingArtifactFailsClosed()
{
    const auto result =
        inspectCompositionConditionedSequenceNeuralModelArtifactFile(
            "__midi_gengx_missing_model__.mgcn");

    expect(
        !result.isValid(),
        "missing artifact fails closed");
}

} // namespace

int main()
{
    testArtifactFileRoundTrip();
    testMissingArtifactFailsClosed();

    std::cout
        << "MIDI-GenGX Phase 113 real model artifact inspector tests passed.\n";

    return 0;
}
