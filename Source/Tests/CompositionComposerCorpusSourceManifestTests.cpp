#include "Music/CompositionComposerCorpusSourceManifest.h"

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

CompositionComposerKnowledgeSample makeSample(
    const std::string& id,
    const std::string& composer,
    const std::string& work)
{
    CompositionDatasetSample composition;
    composition.sampleId = id;
    composition.globalFeatures =
    {
        0.25,
        0.50,
        0.25
    };
    composition.sectionFeatures =
    {
        {
            0.0,
            0.2,
            0.0,
            0.25,
            0.0,
            0.0
        }
    };
    composition.analysisValid = true;

    CompositionSequenceMetadata metadata;
    metadata.sampleId = id;
    metadata.composerId = composer;
    metadata.workId = work;
    metadata.movementId = id;
    metadata.styleId = "style";
    metadata.eraId = "era";
    metadata.instrumentationId = "solo_piano";
    metadata.verified = true;
    metadata.valid = true;

    return buildCompositionComposerKnowledgeSample(
        composition,
        metadata);
}

CompositionComposerKnowledgeCatalog makeCatalog()
{
    return buildCompositionComposerKnowledgeCatalog(
    {
        makeSample(
            "bach-piece-a",
            "bach",
            "piece-a"),
        makeSample(
            "chopin-piece-b",
            "chopin",
            "piece-b"),
        makeSample(
            "Nested/chopin-piece-c",
            "chopin",
            "piece-c")
    });
}

fs::path createCorpus()
{
    const auto root =
        fs::temp_directory_path() /
        "MIDI_GenGX_Phase101_Source";

    std::error_code error;

    fs::remove_all(
        root,
        error);

    fs::create_directories(
        root / "Nested",
        error);

    const auto files =
    {
        root / "bach-piece-a.mid",
        root / "chopin-piece-b.midi",
        root / "Nested" / "chopin-piece-c.mid"
    };

    std::uint8_t seed = 1;

    for (const auto& file :
         files)
    {
        std::ofstream output(
            file,
            std::ios::binary);

        output.put(
            static_cast<char>(0x4D));

        output.put(
            static_cast<char>(0x54));

        output.put(
            static_cast<char>(seed++));
    }

    return root;
}

void testRealDirectoryMapsExactlyToCatalog()
{
    const auto root =
        createCorpus();

    const auto catalog =
        makeCatalog();

    const auto manifest =
        buildCompositionComposerCorpusSourceManifest(
            root.string(),
            catalog,
            true);

    expect(
        manifest.isValid(),
        "real MIDI source manifest is valid");

    expect(
        manifest.sampleCount() ==
            catalog.sampleCount(),
        "source manifest contains every catalog sample");

    expect(
        manifest.entries[0].sampleId ==
            "bach-piece-a",
        "top-level MIDI ID is stable");

    expect(
        manifest.entries[2].sampleId ==
            "Nested/chopin-piece-c",
        "nested MIDI ID is stable");

    for (const auto& entry :
         manifest.entries)
    {
        expect(
            entry.byteSize > 0 &&
            entry.contentHash != 0,
            "source file fingerprint is populated");
    }

    std::error_code error;
    fs::remove_all(root,error);
}

void testMissingFileFailsClosed()
{
    const auto root =
        createCorpus();

    fs::remove(
        root / "chopin-piece-b.midi");

    const auto manifest =
        buildCompositionComposerCorpusSourceManifest(
            root.string(),
            makeCatalog(),
            true);

    expect(
        !manifest.isValid(),
        "missing MIDI source fails manifest creation");

    std::error_code error;
    fs::remove_all(root,error);
}

void testUnexpectedMidiFailsClosed()
{
    const auto root =
        createCorpus();

    std::ofstream extra(
        root / "unexpected.mid",
        std::ios::binary);

    extra.put(
        static_cast<char>(0x01));

    const auto manifest =
        buildCompositionComposerCorpusSourceManifest(
            root.string(),
            makeCatalog(),
            true);

    expect(
        !manifest.isValid(),
        "unexpected MIDI source fails manifest creation");

    std::error_code error;
    fs::remove_all(root,error);
}

void testNonRecursiveModeRespectsRootBoundary()
{
    const auto root =
        createCorpus();

    const auto manifest =
        buildCompositionComposerCorpusSourceManifest(
            root.string(),
            makeCatalog(),
            false);

    expect(
        !manifest.isValid(),
        "non-recursive mode rejects nested catalog sources");

    std::error_code error;
    fs::remove_all(root,error);
}

} // namespace

int main()
{
    testRealDirectoryMapsExactlyToCatalog();
    testMissingFileFailsClosed();
    testUnexpectedMidiFailsClosed();
    testNonRecursiveModeRespectsRootBoundary();

    std::cout
        << "MIDI-GenGX Phase 101 composer corpus source manifest tests passed.\n";

    return 0;
}
