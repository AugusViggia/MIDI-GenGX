#include "Music/CompositionComposerKnowledgeCorpusAssembly.h"

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
    composition.globalFeatures = {0.25, 0.50, 0.25};
    composition.sectionFeatures =
    {
        {0.0, 0.2, 0.0, 0.25, 0.0, 0.0}
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
        makeSample("bach-w1", "bach", "w1"),
        makeSample("bach-w2", "bach", "w2"),
        makeSample("chopin-w1", "chopin", "w1"),
        makeSample("chopin-w2", "chopin", "w2"),
        makeSample("Nested/chopin-w3", "chopin", "w3")
    });
}

fs::path createSourceTree()
{
    const auto root =
        fs::temp_directory_path() /
        "MIDI_GenGX_Phase102_Source";

    std::error_code error;

    fs::remove_all(root, error);
    fs::create_directories(
        root / "Nested",
        error);

    const auto files =
    {
        root / "bach-w1.mid",
        root / "bach-w2.mid",
        root / "chopin-w1.mid",
        root / "chopin-w2.mid",
        root / "Nested" / "chopin-w3.mid"
    };

    std::uint8_t byte = 1;

    for (const auto& path : files)
    {
        std::ofstream output(
            path,
            std::ios::binary);

        output.put(
            static_cast<char>(
                0x4D));

        output.put(
            static_cast<char>(
                0x54));

        output.put(
            static_cast<char>(
                byte++));
    }

    return root;
}

void testRealSourceAndTrainingCorpusAssemble()
{
    const auto root =
        createSourceTree();

    const auto catalog =
        makeCatalog();

    const auto partition =
        buildCompositionComposerKnowledgePartition(
            catalog,
            0.2,
            0.2);

    const auto manifest =
        buildCompositionComposerKnowledgeCorpusManifest(
            catalog,
            partition,
            "phase102",
            "1");

    const auto trainingCorpus =
        buildCompositionComposerKnowledgeTrainingCorpus(
            catalog,
            partition,
            manifest);

    const auto sourceManifest =
        buildCompositionComposerCorpusSourceManifest(
            root.string(),
            catalog,
            true);

    const auto assembly =
        assembleCompositionComposerKnowledgeCorpus(
            sourceManifest,
            trainingCorpus);

    expect(
        assembly.isValid(),
        "real source manifest and training corpus assemble");

    expect(
        assembly.sampleCount() ==
            catalog.sampleCount(),
        "assembly preserves corpus cardinality");

    std::error_code error;
    fs::remove_all(root, error);
}

void testMissingTrainingSampleFailsClosed()
{
    const auto root =
        createSourceTree();

    const auto catalog =
        makeCatalog();

    const auto partition =
        buildCompositionComposerKnowledgePartition(
            catalog,
            0.2,
            0.2);

    auto manifest =
        buildCompositionComposerKnowledgeCorpusManifest(
            catalog,
            partition,
            "phase102",
            "1");

    auto trainingCorpus =
        buildCompositionComposerKnowledgeTrainingCorpus(
            catalog,
            partition,
            manifest);

    trainingCorpus.trainingSamples.pop_back();

    const auto sourceManifest =
        buildCompositionComposerCorpusSourceManifest(
            root.string(),
            catalog,
            true);

    const auto assembly =
        assembleCompositionComposerKnowledgeCorpus(
            sourceManifest,
            trainingCorpus);

    expect(
        !assembly.isValid(),
        "missing training sample fails assembly");

    std::error_code error;
    fs::remove_all(root, error);
}

void testUnknownSourceSampleFailsClosed()
{
    const auto root =
        createSourceTree();

    const auto catalog =
        makeCatalog();

    auto sourceManifest =
        buildCompositionComposerCorpusSourceManifest(
            root.string(),
            catalog,
            true);

    expect(
        sourceManifest.isValid(),
        "source manifest fixture is valid");

    sourceManifest.entries[0].sampleId =
        "unexpected";

    const auto partition =
        buildCompositionComposerKnowledgePartition(
            catalog,
            0.2,
            0.2);

    const auto manifest =
        buildCompositionComposerKnowledgeCorpusManifest(
            catalog,
            partition,
            "phase102",
            "1");

    const auto trainingCorpus =
        buildCompositionComposerKnowledgeTrainingCorpus(
            catalog,
            partition,
            manifest);

    const auto assembly =
        assembleCompositionComposerKnowledgeCorpus(
            sourceManifest,
            trainingCorpus);

    expect(
        !assembly.isValid(),
        "unknown source sample fails assembly");

    std::error_code error;
    fs::remove_all(root, error);
}

} // namespace

int main()
{
    testRealSourceAndTrainingCorpusAssemble();
    testMissingTrainingSampleFailsClosed();
    testUnknownSourceSampleFailsClosed();

    std::cout
        << "MIDI-GenGX Phase 102 corpus assembly tests passed.\n";

    return 0;
}
