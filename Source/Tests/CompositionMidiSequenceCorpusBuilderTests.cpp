#include "Music/CompositionMidiSequenceCorpusBuilder.h"
#include "Music/CompositionMidiCorpusDirectoryLoader.h"
#include "Music/CompositionMidiSectionAnalyzer.h"
#include "Music/CompositionMidiTrainingSequence.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <vector>

using namespace midigengx::music;

namespace
{
namespace fs = std::filesystem;

void expect(bool condition, const char* message)
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

std::vector<std::uint8_t>
buildMidi(
    int root)
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

    constexpr int chord[3] =
    {
        0,4,7
    };

    std::vector<std::uint8_t> track;

    for (int beat = 0;
         beat < 64;
         ++beat)
    {
        for (int note = 0;
             note < 3;
             ++note)
        {
            track.push_back(0x00);
            track.push_back(0x90);
            track.push_back(
                static_cast<std::uint8_t>(
                    root + chord[note]));
            track.push_back(96);
        }

        for (int note = 0;
             note < 3;
             ++note)
        {
            if (note == 0)
            {
                track.push_back(0x83);
                track.push_back(0x60);
            }
            else
            {
                track.push_back(0x00);
            }

            track.push_back(0x80);
            track.push_back(
                static_cast<std::uint8_t>(
                    root + chord[note]));
            track.push_back(0);
        }
    }

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
        "MIDI_GenGX_Phase88_Corpus";

    std::error_code error;
    fs::remove_all(
        root,
        error);

    fs::create_directories(
        root / "Nested",
        error);

    for (std::size_t index = 0;
         index < count;
         ++index)
    {
        const auto bytes =
            buildMidi(
                60 +
                static_cast<int>(
                    index % 5));

        const auto filePath =
            index % 2 == 0
                ? root /
                  ("piece-" +
                   std::to_string(index) +
                   ".mid")
                : root /
                  "Nested" /
                  ("piece-" +
                   std::to_string(index) +
                   ".midi");

        std::ofstream file(
            filePath,
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

void testBuildsSequenceCorpus()
{
    const auto root =
        createCorpus(6);

    const auto load =
        loadCompositionMidiCorpusDirectory(
            root.string(),
            true);

    expect(
        load.isValid(),
        "MIDI directory loader is valid");

    expect(
        load.acceptedFileCount == 6,
        "MIDI loader accepts all six fixture files");

    expect(
        load.rejectedFileCount == 0,
        "MIDI loader rejects none of the valid fixture files");

    const auto result =
        buildCompositionMidiSequenceCorpusFromDirectory(
            root.string(),
            true);

    expect(
        result.isValid(),
        "real MIDI directory becomes a valid sequence corpus");

    expect(
        result.inputFileCount == 6,
        "all MIDI files are discovered");

    expect(
        result.sequenceCount == 6,
        "all valid MIDI files become training sequences");

    expect(
        result.rejectedCount == 0,
        "no valid fixture sequence is rejected");

    expect(
        result.eventCount > 0,
        "sequence corpus contains real note events");

    std::vector<CompositionMidiTrainingSequence>
        restored;

    expect(
        deserializeCompositionMidiTrainingSequences(
            result.artifact,
            restored),
        "sequence corpus artifact can be decoded");

    expect(
        restored.size() == 6,
        "artifact preserves sequence count");

    cleanup(root);
}

void testNestedIdsAreDeterministic()
{
    const auto root =
        createCorpus(4);

    const auto first =
        buildCompositionMidiSequenceCorpusFromDirectory(
            root.string(),
            true);

    const auto second =
        buildCompositionMidiSequenceCorpusFromDirectory(
            root.string(),
            true);

    expect(
        first.isValid() &&
        second.isValid(),
        "repeated corpus builds are valid");

    expect(
        first.artifact.bytes ==
            second.artifact.bytes,
        "sequence corpus artifact is deterministic");

    cleanup(root);
}

void testMalformedFileIsRejected()
{
    const auto root =
        createCorpus(3);

    std::ofstream invalid(
        root / "Broken.mid",
        std::ios::binary);

    invalid.write(
        "NOPE",
        4);

    const auto result =
        buildCompositionMidiSequenceCorpusFromDirectory(
            root.string(),
            false);

    expect(
        result.isValid(),
        "malformed-file corpus build remains structurally valid");

    expect(
        result.inputFileCount == 3,
        "malformed MIDI is included in discovered files");

    expect(
        result.sequenceCount == 2,
        "valid MIDI files are converted to sequences");

    expect(
        result.rejectedCount == 1,
        "malformed MIDI is accounted for at the sequence boundary");

    cleanup(root);
}

void testEmptyDirectoryIsRejected()
{
    const auto root =
        fs::temp_directory_path() /
        "MIDI_GenGX_Phase88_Empty";

    std::error_code error;
    fs::remove_all(root,error);
    fs::create_directories(root,error);

    const auto result =
        buildCompositionMidiSequenceCorpusFromDirectory(
            root.string(),
            true);

    expect(
        !result.valid,
        "empty sequence corpus is not training-ready");

    cleanup(root);
}

void testDeterministicOrderingIsStable()
{
    const auto root =
        createCorpus(6);

    const auto result =
        buildCompositionMidiSequenceCorpusFromDirectory(
            root.string(),
            true);

    expect(
        result.isValid(),
        "ordering test corpus is valid");

    std::vector<CompositionMidiTrainingSequence>
        restored;

    expect(
        deserializeCompositionMidiTrainingSequences(
            result.artifact,
            restored),
        "ordering test artifact decodes");

    expect(
        restored.size() == 6,
        "ordering test preserves all sequences");

    for (std::size_t index = 1;
         index < restored.size();
         ++index)
    {
        expect(
            restored[index - 1].sampleId <
                restored[index].sampleId,
            "sequence IDs are sorted deterministically");
    }

    cleanup(root);
}

} // namespace

int main()
{
    testBuildsSequenceCorpus();
    testNestedIdsAreDeterministic();
    testMalformedFileIsRejected();
    testEmptyDirectoryIsRejected();
    testDeterministicOrderingIsStable();

    std::cout
        << "MIDI-GenGX Phase 88 sequence corpus builder tests passed.\n";

    return 0;
}
