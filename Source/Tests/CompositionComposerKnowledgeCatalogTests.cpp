#include "Music/CompositionComposerKnowledgeCatalog.h"

#include <cstdlib>
#include <iostream>

using namespace midigengx::music;

namespace
{

void expect(
    bool condition,
    const char* message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}

CompositionDatasetSample makeComposition(
    const std::string& id)
{
    CompositionDatasetSample sample;
    sample.sampleId = id;
    sample.globalFeatures = {0.25, 0.50, 0.25};
    sample.sectionFeatures =
    {
        {0.0, 0.2, 0.0, 0.25, 0.0, 0.0}
    };
    sample.analysisValid = true;
    return sample;
}

CompositionSequenceMetadata makeMetadata(
    const std::string& id,
    const std::string& composer)
{
    CompositionSequenceMetadata metadata;
    metadata.sampleId = id;
    metadata.composerId = composer;
    metadata.workId = id + "_work";
    metadata.movementId = "single";
    metadata.styleId = "learned_style";
    metadata.eraId = "learned_era";
    metadata.instrumentationId = "solo_piano";
    metadata.verified = true;
    metadata.valid = true;
    return metadata;
}

CompositionComposerKnowledgeSample makeSample(
    const std::string& id,
    const std::string& composer)
{
    return buildCompositionComposerKnowledgeSample(
        makeComposition(id),
        makeMetadata(id, composer));
}

void testCatalogGroupsByComposer()
{
    const auto catalog =
        buildCompositionComposerKnowledgeCatalog(
        {
            makeSample("bach-b", "bach"),
            makeSample("chopin-b", "chopin"),
            makeSample("chopin-a", "chopin"),
            makeSample("bach-a", "bach")
        });

    expect(
        catalog.isValid(),
        "composer knowledge catalog is valid");

    expect(
        catalog.composerCount() == 2 &&
        catalog.sampleCount() == 4,
        "all composer groups and samples are preserved");

    expect(
        catalog.composers[0].composerId == "bach" &&
        catalog.composers[1].composerId == "chopin",
        "composer groups are deterministic");

    expect(
        catalog.composers[1].samples[0].composition.sampleId ==
            "chopin-a",
        "samples inside composer groups are deterministic");
}

void testComposerLookup()
{
    const auto catalog =
        buildCompositionComposerKnowledgeCatalog(
        {
            makeSample("a", "bach"),
            makeSample("b", "chopin")
        });

    const auto* chopin =
        catalog.findComposer("chopin");

    expect(
        chopin != nullptr &&
        chopin->sampleCount() == 1,
        "composer lookup returns the correct group");

    expect(
        catalog.findComposer("mozart") == nullptr,
        "unknown composer lookup is rejected");
}

void testUnverifiedMetadataFailsByDefault()
{
    auto sample =
        makeSample("a", "chopin");

    sample.metadata.verified = false;

    const auto catalog =
        buildCompositionComposerKnowledgeCatalog(
        {
            sample
        });

    expect(
        !catalog.isValid(),
        "unverified composer corpus is rejected by default");
}

void testUnverifiedMetadataCanBeRepresented()
{
    auto sample =
        makeSample("a", "chopin");

    sample.metadata.verified = false;

    const auto catalog =
        buildCompositionComposerKnowledgeCatalog(
        {
            sample
        },
        false);

    expect(
        catalog.isValid() &&
        !catalog.composers[0].samples[0].metadata.verified,
        "explicit structural mode preserves verification state");
}

} // namespace

int main()
{
    testCatalogGroupsByComposer();
    testComposerLookup();
    testUnverifiedMetadataFailsByDefault();
    testUnverifiedMetadataCanBeRepresented();

    std::cout
        << "MIDI-GenGX Phase 97 composer knowledge catalog tests passed.\n";

    return 0;
}
