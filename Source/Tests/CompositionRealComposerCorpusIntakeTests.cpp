#include "Music/CompositionRealComposerCorpusIntake.h"

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
            "chopin-piece-a",
            "chopin",
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
        "MIDI_GenGX_Phase103_RealCorpus";

    std::error_code error;

    fs::remove_all(
        root,
        error);

    fs::create_directories(
        root / "Nested",
        error);

    for (const auto& path :
         {
             root / "chopin-piece-a.mid",
             root / "chopin-piece-b.mid",
             root / "Nested" / "chopin-piece-c.mid"
         })
    {
        std::ofstream output(
            path,
            std::ios::binary);

        output.write(
            "MThd",
            4);
    }

    return root;
}

void testRealComposerIntakeMatchesCatalog()
{
    const auto root =
        createCorpus();

    const auto report =
        inspectRealComposerCorpusDirectory(
            root.string(),
            "chopin",
            makeCatalog(),
            true);

    expect(
        report.isValid(),
        "real composer corpus intake is valid");

    expect(
        report.expectedSamples == 3 &&
        report.matchedSamples == 3,
        "all real composer samples match");

    expect(
        canEnterFirstComposerTraining(
            report,
            [&]()
            {
                CompositionComposerKnowledgeCorpusAssembly unused;
                return unused;
            }()) == false,
        "unassembled corpus cannot enter training");

    std::error_code error;
    fs::remove_all(
        root,
        error);
}

void testMissingComposerFileFailsClosed()
{
    const auto root =
        createCorpus();

    fs::remove(
        root / "chopin-piece-b.mid");

    const auto report =
        inspectRealComposerCorpusDirectory(
            root.string(),
            "chopin",
            makeCatalog(),
            true);

    expect(
        !report.isValid(),
        "missing real composer MIDI fails closed");

    expect(
        !report.issues.empty(),
        "missing source produces an intake issue");

    std::error_code error;
    fs::remove_all(
        root,
        error);
}

void testUnexpectedComposerFileFailsClosed()
{
    const auto root =
        createCorpus();

    std::ofstream extra(
        root / "unknown-piece.mid",
        std::ios::binary);

    extra.write(
        "MThd",
        4);

    const auto report =
        inspectRealComposerCorpusDirectory(
            root.string(),
            "chopin",
            makeCatalog(),
            true);

    expect(
        !report.isValid(),
        "unexpected real composer MIDI fails closed");

    expect(
        !report.issues.empty(),
        "unexpected source produces an intake issue");

    std::error_code error;
    fs::remove_all(
        root,
        error);
}

void testUnknownComposerFailsClosed()
{
    const auto report =
        inspectRealComposerCorpusDirectory(
            "C:/does-not-matter",
            "mozart",
            makeCatalog(),
            true);

    expect(
        !report.isValid(),
        "unknown composer intake fails closed");
}

} // namespace

int main()
{
    testRealComposerIntakeMatchesCatalog();
    testMissingComposerFileFailsClosed();
    testUnexpectedComposerFileFailsClosed();
    testUnknownComposerFailsClosed();

    std::cout
        << "MIDI-GenGX Phase 103 real composer corpus intake tests passed.\n";

    return 0;
}
