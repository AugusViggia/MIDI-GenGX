#include "Music/CompositionComposerKnowledgeSample.h"

#include <algorithm>
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

CompositionDatasetSample makeComposition(
    const std::string& id)
{
    CompositionDatasetSample sample;

    sample.sampleId =
        id;

    sample.globalFeatures =
    {
        0.25,
        0.50,
        0.25
    };

    sample.sectionFeatures =
    {
        {
            0.0,
            0.2,
            0.0,
            0.25,
            0.0,
            0.0
        },
        {
            0.33,
            0.6,
            0.4,
            0.50,
            0.2,
            0.25
        }
    };

    sample.analysisValid =
        true;

    return sample;
}

CompositionSequenceMetadata makeMetadata(
    const std::string& id)
{
    CompositionSequenceMetadata metadata;

    metadata.sampleId =
        id;

    metadata.composerId =
        "chopin";

    metadata.workId =
        "op9-no2";

    metadata.movementId =
        "single";

    metadata.styleId =
        "romantic";

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

void testKnowledgeSampleJoinsAnalysisAndMetadata()
{
    const auto composition =
        makeComposition(
            "op9-no2");

    const auto metadata =
        makeMetadata(
            "op9-no2");

    const auto sample =
        buildCompositionComposerKnowledgeSample(
            composition,
            metadata);

    expect(
        sample.isValid(),
        "composer knowledge sample is valid");

    expect(
        sample.metadata.composerId ==
            "chopin",
        "composer identity is preserved");

    expect(
        sample.composition.sectionCount() ==
            2,
        "musical composition representation is preserved");
}

void testMismatchedIdentityIsRejected()
{
    const auto composition =
        makeComposition(
            "op9-no2");

    const auto metadata =
        makeMetadata(
            "different-piece");

    const auto sample =
        buildCompositionComposerKnowledgeSample(
            composition,
            metadata);

    expect(
        !sample.isValid(),
        "composition and metadata with different sample IDs are rejected");
}

void testUnverifiedMetadataIsStructurallyRepresentable()
{
    auto metadata =
        makeMetadata(
            "op9-no2");

    metadata.verified =
        false;

    const auto sample =
        buildCompositionComposerKnowledgeSample(
            makeComposition(
                "op9-no2"),
            metadata);

    expect(
        sample.isValid(),
        "unverified metadata remains structurally representable");

    expect(
        !sample.metadata.verified,
        "verification status is preserved without being rewritten");
}

} // namespace

int main()
{
    testKnowledgeSampleJoinsAnalysisAndMetadata();
    testMismatchedIdentityIsRejected();
    testUnverifiedMetadataIsStructurallyRepresentable();

    std::cout
        << "MIDI-GenGX Phase 96 composer knowledge representation tests passed.\n";

    return 0;
}
