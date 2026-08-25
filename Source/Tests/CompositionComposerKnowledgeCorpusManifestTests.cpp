#include "Music/CompositionComposerKnowledgeCorpusManifest.h"

#include <cstdlib>
#include <iostream>
#include <string>

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
        makeSample("bach-w1-a", "bach", "w1"),
        makeSample("bach-w2", "bach", "w2"),
        makeSample("bach-w3", "bach", "w3"),
        makeSample("chopin-w1", "chopin", "w1"),
        makeSample("chopin-w2", "chopin", "w2"),
        makeSample("chopin-w3", "chopin", "w3"),
        makeSample("chopin-w4", "chopin", "w4")
    });
}

void testManifestIsValid()
{
    const auto catalog =
        makeCatalog();

    const auto partition =
        buildCompositionComposerKnowledgePartition(
            catalog,
            0.25,
            0.25);

    const auto manifest =
        buildCompositionComposerKnowledgeCorpusManifest(
            catalog,
            partition,
            "classical-composers",
            "2026.08.24");

    expect(
        manifest.isValid(),
        "composer knowledge corpus manifest is valid");

    expect(
        manifest.composerCount == 2 &&
        manifest.sampleCount == 7,
        "manifest preserves corpus cardinalities");

    expect(
        manifest.workCount == 7,
        "manifest counts unique composer/work pairs");

    expect(
        manifest.trainingCount +
            manifest.validationCount +
            manifest.testCount ==
            manifest.sampleCount,
        "manifest split counts cover the corpus");
}

void testManifestPreservesComposerIdentity()
{
    const auto catalog =
        makeCatalog();

    const auto partition =
        buildCompositionComposerKnowledgePartition(
            catalog,
            0.25,
            0.25);

    const auto manifest =
        buildCompositionComposerKnowledgeCorpusManifest(
            catalog,
            partition,
            "classical-composers",
            "2026.08.24");

    expect(
        manifest.composerIds.size() == 2 &&
        manifest.composerIds[0] == "bach" &&
        manifest.composerIds[1] == "chopin",
        "manifest stores deterministic composer identities");
}

void testInvalidInputsFailClosed()
{
    const auto catalog =
        makeCatalog();

    CompositionComposerKnowledgePartition invalid;

    const auto manifest =
        buildCompositionComposerKnowledgeCorpusManifest(
            catalog,
            invalid,
            "classical-composers",
            "2026.08.24");

    expect(
        !manifest.isValid(),
        "invalid partition fails manifest creation");

    const auto validPartition =
        buildCompositionComposerKnowledgePartition(
            catalog,
            0.25,
            0.25);

    const auto missingIdentity =
        buildCompositionComposerKnowledgeCorpusManifest(
            catalog,
            validPartition,
            "",
            "2026.08.24");

    expect(
        !missingIdentity.isValid(),
        "missing corpus identity fails manifest creation");
}

void testUnverifiedCatalogCannotProduceVerifiedManifest()
{
    auto sample =
        makeSample(
            "chopin-w1",
            "chopin",
            "w1");

    sample.metadata.verified =
        false;

    const auto catalog =
        buildCompositionComposerKnowledgeCatalog(
        {
            sample
        },
        false);

    expect(
        catalog.isValid() &&
        !catalog.verified,
        "structural catalog preserves unverified state");

    const auto partition =
        buildCompositionComposerKnowledgePartition(
            catalog,
            0.0,
            0.0);

    const auto manifest =
        buildCompositionComposerKnowledgeCorpusManifest(
            catalog,
            partition,
            "unverified",
            "1");

    expect(
        !manifest.isValid(),
        "unverified catalog cannot produce a verified manifest");
}

} // namespace

int main()
{
    testManifestIsValid();
    testManifestPreservesComposerIdentity();
    testInvalidInputsFailClosed();
    testUnverifiedCatalogCannotProduceVerifiedManifest();

    std::cout
        << "MIDI-GenGX Phase 99 composer knowledge corpus manifest tests passed.\n";

    return 0;
}
