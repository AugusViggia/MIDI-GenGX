#include "Music/CompositionSequenceMetadataFileLoader.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>

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

fs::path writeValidMetadata()
{
    const auto path =
        fs::temp_directory_path() /
        "MIDI_GenGX_Phase109_Metadata.tsv";

    std::ofstream file(path);

    file
        << "# sampleId\tcomposerId\tworkId\tmovementId\tstyleId\teraId\tinstrumentationId\tverified\n"
        << "chopin-piece-a\tchopin\twork-a\tmovement-1\tromantic_piano\tromantic\tsolo_piano\t1\n"
        << "chopin-piece-b\tchopin\twork-b\tmovement-1\tromantic_piano\tromantic\tsolo_piano\ttrue\n";
    file.close();

    return path;
}

void testValidMetadataFile()
{
    const auto path =
        writeValidMetadata();

    const auto result =
        loadCompositionSequenceMetadataFile(
            path.string());

    expect(
        result.isValid(),
        "valid metadata file loads");

    expect(
        result.parsedEntryCount == 2 &&
        result.catalog.entries.size() == 2,
        "all metadata rows are preserved");

    expect(
        result.catalog.entries[0].sampleId ==
            "chopin-piece-a" &&
        result.catalog.verified,
        "catalog identity and verification are preserved");

    std::error_code error;
    fs::remove(path,error);
}

void testRejectedLineFailsClosed()
{
    const auto path =
        fs::temp_directory_path() /
        "MIDI_GenGX_Phase109_InvalidMetadata.tsv";

    std::ofstream file(path);

    file
        << "chopin-piece-a\tchopin\twork-a\tmovement-1\tromantic_piano\tromantic\tsolo_piano\t1\n"
        << "broken-row\tchopin\ttoo-few-fields\n";
    file.close();

    const auto result =
        loadCompositionSequenceMetadataFile(
            path.string());

    expect(
        !result.isValid(),
        "rejected metadata line invalidates the file");

    expect(
        result.rejectedLineCount == 1,
        "rejected line is counted");

    std::error_code error;
    fs::remove(path,error);
}

void testDuplicateIdsFailClosed()
{
    const auto path =
        fs::temp_directory_path() /
        "MIDI_GenGX_Phase109_DuplicateMetadata.tsv";

    std::ofstream file(path);

    file
        << "same\tchopin\twork-a\tmovement-1\tromantic_piano\tromantic\tsolo_piano\t1\n"
        << "same\tchopin\twork-b\tmovement-1\tromantic_piano\tromantic\tsolo_piano\t1\n";
    file.close();

    const auto result =
        loadCompositionSequenceMetadataFile(
            path.string());

    expect(
        !result.isValid(),
        "duplicate sample IDs fail closed");

    std::error_code error;
    fs::remove(path,error);
}

} // namespace

int main()
{
    testValidMetadataFile();
    testRejectedLineFailsClosed();
    testDuplicateIdsFailClosed();

    std::cout
        << "MIDI-GenGX Phase 109 metadata file loader tests passed.\n";

    return 0;
}
