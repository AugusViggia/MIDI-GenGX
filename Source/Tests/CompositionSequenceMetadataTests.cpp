#include "Music/CompositionSequenceMetadataArtifact.h"

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
        std::cerr
            << "FAILED: "
            << message
            << '\n';
        std::exit(1);
    }
}

CompositionSequenceMetadata makeMetadata(
    const std::string& sampleId,
    const std::string& composer)
{
    CompositionSequenceMetadata metadata;

    metadata.sampleId =
        sampleId;

    metadata.composerId =
        composer;

    metadata.workId =
        "work_" +
        sampleId;

    metadata.movementId =
        "movement_1";

    metadata.styleId =
        "romantic_piano";

    metadata.eraId =
        "romantic";

    metadata.instrumentationId =
        "solo_piano";

    metadata.verified =
        true;

    metadata.valid =
        true;

    return metadata;
}

void testMetadataValidity()
{
    const auto metadata =
        makeMetadata(
            "piece-a",
            "chopin");

    expect(
        metadata.isValid(),
        "complete metadata is valid");
}

void testCatalogSortsAndRejectsDuplicates()
{
    const auto valid =
        makeMetadata(
            "piece-b",
            "bach");

    const auto valid2 =
        makeMetadata(
            "piece-a",
            "mozart");

    const auto catalog =
        buildCompositionSequenceMetadataCatalog(
        {
            valid,
            valid2
        });

    expect(
        catalog.isValid(),
        "metadata catalog is valid");

    expect(
        catalog.entries[0].sampleId ==
            "piece-a" &&
        catalog.entries[1].sampleId ==
            "piece-b",
        "metadata catalog ordering is deterministic");

    const auto duplicate =
        buildCompositionSequenceMetadataCatalog(
        {
            valid,
            valid
        });

    expect(
        !duplicate.isValid(),
        "duplicate metadata sample IDs are rejected");
}

void testCatalogArtifactRoundTrip()
{
    const auto catalog =
        buildCompositionSequenceMetadataCatalog(
        {
            makeMetadata(
                "piece-a",
                "chopin"),
            makeMetadata(
                "piece-b",
                "bach")
        });

    const auto artifact =
        serializeCompositionSequenceMetadataCatalog(
            catalog);

    expect(
        artifact.isValid(),
        "metadata artifact is valid");

    CompositionSequenceMetadataCatalog restored;

    expect(
        deserializeCompositionSequenceMetadataCatalog(
            artifact,
            restored),
        "metadata artifact round trip succeeds");

    expect(
        restored.entries.size() ==
            catalog.entries.size(),
        "round trip preserves metadata count");

    expect(
        restored.entries[0].composerId ==
            "chopin" &&
        restored.entries[1].composerId ==
            "bach",
        "round trip preserves composer metadata");
}

void testUnverifiedMetadataIsNotTrainingReady()
{
    auto metadata =
        makeMetadata(
            "piece-a",
            "chopin");

    metadata.verified =
        false;

    const auto catalog =
        buildCompositionSequenceMetadataCatalog(
        {
            metadata
        });

    expect(
        catalog.isValid(),
        "unverified metadata can exist as a valid catalog entry");

    expect(
        !catalog.verified,
        "catalog explicitly reports unverified metadata");

    const auto artifact =
        serializeCompositionSequenceMetadataCatalog(
            catalog);

    expect(
        artifact.isValid(),
        "unverified metadata can be persisted without being mislabeled verified");
}

void testSampleLookup()
{
    const auto catalog =
        buildCompositionSequenceMetadataCatalog(
        {
            makeMetadata(
                "piece-a",
                "chopin")
        });

    const auto* metadata =
        catalog.findBySampleId(
            "piece-a");

    expect(
        metadata != nullptr,
        "metadata lookup finds sample by stable ID");

    expect(
        metadata->composerId ==
            "chopin",
        "sample lookup returns correct composer");
}

} // namespace

int main()
{
    testMetadataValidity();
    testCatalogSortsAndRejectsDuplicates();
    testCatalogArtifactRoundTrip();
    testUnverifiedMetadataIsNotTrainingReady();
    testSampleLookup();

    std::cout
        << "MIDI-GenGX Phase 89 metadata tests passed.\n";

    return 0;
}
