#include "Music/CompositionMidiTrainingPipeline.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <vector>

using namespace midigengx::music;

namespace
{

namespace fs = std::filesystem;

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

void appendU32BE(
    std::vector<std::uint8_t>& bytes,
    std::uint32_t value)
{
    bytes.push_back(
        static_cast<std::uint8_t>(
            value >> 24));
    bytes.push_back(
        static_cast<std::uint8_t>(
            value >> 16));
    bytes.push_back(
        static_cast<std::uint8_t>(
            value >> 8));
    bytes.push_back(
        static_cast<std::uint8_t>(
            value));
}

std::vector<std::uint8_t> buildMidi(
    int pitch)
{
    std::vector<std::uint8_t> bytes =
    {
        'M','T','h','d'
    };

    appendU32BE(
        bytes,
        6);

    bytes.push_back(0);
    bytes.push_back(0);

    bytes.push_back(0);
    bytes.push_back(1);

    bytes.push_back(0x01);
    bytes.push_back(0xE0);

    std::vector<std::uint8_t> track;

    // Build 16 bars of repeated major-triad material. This fixture is
    // intentionally rich enough for every real analysis stage:
    // sections, harmony and motif recurrence.
    constexpr int chord[3] =
    {
        0, 4, 7
    };

    for (int beat = 0;
         beat < 64;
         ++beat)
    {
        for (const auto interval :
             chord)
        {
            track.push_back(
                0x00);
            track.push_back(
                0x90);
            track.push_back(
                static_cast<std::uint8_t>(
                    pitch + interval));
            track.push_back(
                96);
        }

        // Hold the complete triad for one quarter note.
        for (std::size_t noteIndex = 0;
             noteIndex < 3;
             ++noteIndex)
        {
            track.push_back(
                noteIndex == 0
                    ? 0x83
                    : 0x00);

            if (noteIndex == 0)
            {
                track.push_back(
                    0x60);
            }

            track.push_back(
                0x80);

            track.push_back(
                static_cast<std::uint8_t>(
                    pitch +
                    chord[noteIndex]));

            track.push_back(
                0);
        }
    }

    // The final chord already ends at the correct timestamp. End-of-track
    // follows at zero delta without removing any note-off bytes.
    track.push_back(0x00);
    track.push_back(0xFF);
    track.push_back(0x2F);
    track.push_back(0x00);

    bytes.push_back('M');
    bytes.push_back('T');
    bytes.push_back('r');
    bytes.push_back('k');

    appendU32BE(
        bytes,
        static_cast<std::uint32_t>(
            track.size()));

    bytes.insert(
        bytes.end(),
        track.begin(),
        track.end());

    return bytes;
}

fs::path createCorpus(
    std::size_t count)
{
    const auto root =
        fs::temp_directory_path() /
        "MIDI_GenGX_Phase78_Corpus";

    std::error_code error;
    fs::remove_all(root,error);
    fs::create_directories(root,error);

    for (std::size_t index = 0;
         index < count;
         ++index)
    {
        const auto bytes =
            buildMidi(
                60 +
                static_cast<int>(
                    index % 8));

        const auto path =
            root /
            ("sample-" +
             std::to_string(index) +
             ".mid");

        std::ofstream file(
            path,
            std::ios::binary);

        file.write(
            reinterpret_cast<const char*>(
                bytes.data()),
            static_cast<std::streamsize>(
                bytes.size()));
    }

    return root;
}

void cleanup(
    const fs::path& root)
{
    std::error_code error;
    fs::remove_all(
        root,
        error);
}

void testEndToEndTrainingPipeline()
{
    const auto root =
        createCorpus(8);

    const auto result =
        buildCompositionMidiTrainingPipeline(
            root.string(),
            false,
            0.25,
            0.25);

    expect(
        result.isValid(),
        "end-to-end MIDI training pipeline is valid");

    expect(
        result.sampleCount() == 8,
        "all valid MIDI files become dataset samples");

    expect(
        result.quality.sampleCount == 8,
        "dataset quality sees all samples");

    expect(
        result.partition.trainingCount() +
            result.partition.validationCount() +
            result.partition.testCount() ==
            8,
        "partition covers the complete dataset");

    expect(
        result.manifest.sampleCount == 8,
        "manifest records final sample count");

    expect(
        result.prepared.isValid(),
        "prepared learning view is valid");

    expect(
        result.artifact.isValid(),
        "training artifact is valid");

    cleanup(root);
}

void testSmallCorpusStillGetsRequestedSplits()
{
    const auto root =
        createCorpus(3);

    const auto result =
        buildCompositionMidiTrainingPipeline(
            root.string(),
            false,
            0.20,
            0.20);

    expect(
        result.isValid(),
        "small MIDI corpus pipeline is valid");

    expect(
        result.partition.validationCount() == 1 &&
        result.partition.testCount() == 1 &&
        result.partition.trainingCount() == 1,
        "small corpus receives validation/test/training samples");

    cleanup(root);
}

void testInvalidDirectoryFailsPipeline()
{
    const auto root =
        fs::temp_directory_path() /
        "MIDI_GenGX_Phase78_Missing";

    std::error_code error;
    fs::remove_all(root,error);

    const auto result =
        buildCompositionMidiTrainingPipeline(
            root.string(),
            true,
            0.20,
            0.20);

    expect(
        !result.valid,
        "missing corpus directory fails the pipeline");
}

void testEmptyDirectoryIsRejectedAtTrainingBoundary()
{
    const auto root =
        fs::temp_directory_path() /
        "MIDI_GenGX_Phase78_Empty";

    std::error_code error;
    fs::remove_all(root,error);
    fs::create_directories(root,error);

    const auto result =
        buildCompositionMidiTrainingPipeline(
            root.string(),
            true,
            0.20,
            0.20);

    expect(
        !result.valid,
        "empty corpus does not enter the training boundary");

    cleanup(root);
}

void testPipelineIsDeterministic()
{
    const auto root =
        createCorpus(6);

    const auto first =
        buildCompositionMidiTrainingPipeline(
            root.string(),
            false,
            0.20,
            0.20);

    const auto second =
        buildCompositionMidiTrainingPipeline(
            root.string(),
            false,
            0.20,
            0.20);

    expect(
        first.isValid() &&
        second.isValid(),
        "repeated end-to-end pipelines are valid");

    expect(
        first.manifest.datasetSignature ==
            second.manifest.datasetSignature,
        "dataset manifest signature is deterministic");

    expect(
        first.artifact.bytes ==
            second.artifact.bytes,
        "training corpus artifact is deterministic");

    cleanup(root);
}

} // namespace

int main()
{
    testEndToEndTrainingPipeline();
    testSmallCorpusStillGetsRequestedSplits();
    testInvalidDirectoryFailsPipeline();
    testEmptyDirectoryIsRejectedAtTrainingBoundary();
    testPipelineIsDeterministic();

    std::cout
        << "MIDI-GenGX MIDI training pipeline tests passed.\n";

    return 0;
}
