#include "Music/CompositionMidiCorpusDirectoryLoader.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <string>
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

std::vector<std::uint8_t> buildSimpleMidi(
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

    const std::vector<std::uint8_t> track =
    {
        0x00, 0x90,
        static_cast<std::uint8_t>(
            pitch),
        100,
        0x83, 0x60, 0x80,
        static_cast<std::uint8_t>(
            pitch),
        0,
        0x00, 0xFF, 0x2F, 0x00
    };

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

fs::path createFixture()
{
    const auto root =
        fs::temp_directory_path() /
        "MIDI_GenGX_Phase77_Corpus";

    std::error_code error;
    fs::remove_all(
        root,
        error);

    fs::create_directories(
        root / "Nested",
        error);

    std::ofstream(
        root / "Alpha.MID",
        std::ios::binary)
        .write(
            reinterpret_cast<const char*>(
                buildSimpleMidi(60).data()),
            0);

    const auto alpha =
        buildSimpleMidi(60);

    std::ofstream alphaFile(
        root / "Alpha.MID",
        std::ios::binary);

    alphaFile.write(
        reinterpret_cast<const char*>(
            alpha.data()),
        static_cast<std::streamsize>(
            alpha.size()));

    const auto beta =
        buildSimpleMidi(64);

    std::ofstream betaFile(
        root / "Nested" / "Beta.midi",
        std::ios::binary);

    betaFile.write(
        reinterpret_cast<const char*>(
            beta.data()),
        static_cast<std::streamsize>(
            beta.size()));

    std::ofstream invalid(
        root / "Broken.mid",
        std::ios::binary);

    invalid.write(
        "NOPE",
        4);

    std::ofstream ignored(
        root / "readme.txt",
        std::ios::binary);

    ignored << "ignored";

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

void testNonRecursiveDiscovery()
{
    const auto root =
        createFixture();

    const auto result =
        loadCompositionMidiCorpusDirectory(
            root.string(),
            false);

    expect(
        result.isValid(),
        "non-recursive directory load is valid");

    expect(
        result.discoveredFileCount == 2,
        "non-recursive mode discovers only root MIDI files");

    expect(
        result.acceptedFileCount == 1,
        "non-recursive mode accepts valid root MIDI files");

    expect(
        result.rejectedFileCount == 1,
        "non-recursive mode rejects malformed MIDI");

    expect(
        result.records.size() == 1 &&
        result.records.front().sampleId ==
            "Alpha",
        "relative sample id is stable");

    cleanup(root);
}

void testRecursiveDiscovery()
{
    const auto root =
        createFixture();

    const auto result =
        loadCompositionMidiCorpusDirectory(
            root.string(),
            true);

    expect(
        result.isValid(),
        "recursive directory load is valid");

    expect(
        result.discoveredFileCount == 3,
        "recursive mode discovers nested MIDI files");

    expect(
        result.acceptedFileCount == 2,
        "recursive mode accepts both valid MIDI files");

    expect(
        result.rejectedFileCount == 1,
        "recursive mode rejects malformed MIDI");

    expect(
        result.records.size() == 2,
        "recursive mode returns two valid records");

    expect(
        result.records[0].sampleId ==
            "Alpha" &&
        result.records[1].sampleId ==
            "Nested/Beta",
        "recursive sample ids are deterministic and relative");

    cleanup(root);
}

void testCaseInsensitiveExtension()
{
    const auto root =
        createFixture();

    const auto result =
        loadCompositionMidiCorpusDirectory(
            root.string(),
            false);

    expect(
        result.records.size() == 1,
        "uppercase .MID extension is accepted");

    cleanup(root);
}

void testMissingDirectoryIsRejected()
{
    const auto missing =
        fs::temp_directory_path() /
        "MIDI_GenGX_Phase77_Missing";

    std::error_code error;
    fs::remove_all(
        missing,
        error);

    const auto result =
        loadCompositionMidiCorpusDirectory(
            missing.string(),
            true);

    expect(
        !result.valid,
        "missing directory is rejected");
}

void testEmptyDirectoryIsValid()
{
    const auto root =
        fs::temp_directory_path() /
        "MIDI_GenGX_Phase77_Empty";

    std::error_code error;
    fs::remove_all(
        root,
        error);

    fs::create_directories(
        root,
        error);

    const auto result =
        loadCompositionMidiCorpusDirectory(
            root.string(),
            true);

    expect(
        result.isValid(),
        "empty corpus directory is structurally valid");

    expect(
        result.records.empty() &&
        result.discoveredFileCount == 0,
        "empty corpus produces no records");

    cleanup(root);
}

void testDeterministicDiscovery()
{
    const auto root =
        createFixture();

    const auto first =
        loadCompositionMidiCorpusDirectory(
            root.string(),
            true);

    const auto second =
        loadCompositionMidiCorpusDirectory(
            root.string(),
            true);

    expect(
        first.isValid() &&
        second.isValid(),
        "repeated directory loads are valid");

    expect(
        first.records.size() ==
            second.records.size(),
        "repeated directory discovery has stable size");

    for (std::size_t index = 0;
         index < first.records.size();
         ++index)
    {
        expect(
            first.records[index].sampleId ==
                second.records[index].sampleId,
            "repeated directory discovery has stable ordering");
    }

    cleanup(root);
}

} // namespace

int main()
{
    testNonRecursiveDiscovery();
    testRecursiveDiscovery();
    testCaseInsensitiveExtension();
    testMissingDirectoryIsRejected();
    testEmptyDirectoryIsValid();
    testDeterministicDiscovery();

    std::cout
        << "MIDI-GenGX MIDI corpus directory loader tests passed.\n";

    return 0;
}
