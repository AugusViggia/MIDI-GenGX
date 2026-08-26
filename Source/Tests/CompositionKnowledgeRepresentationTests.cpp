#include "Music/CompositionKnowledgeRepresentation.h"
#include "Music/CompositionDatasetSchema.h"
#include "Music/MotifDevelopment.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

using namespace midigengx::music;

namespace
{

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

Motif makeMotif()
{
    Motif motif;
    motif.lengthBeats = 2.0;
    motif.notes =
    {
        MotifNote{0.0, 0.5, 0, 0},
        MotifNote{0.5, 0.5, 2, 1},
        MotifNote{1.0, 0.5, 5, -1},
        MotifNote{1.5, 0.5, 3, 0}
    };
    return motif;
}

CompositionKnowledgeSnapshot buildSnapshot()
{
    midigengx::domain::MusicalContext context;
    context.parameters.lengthBars = 16;
    context.parameters.phraseLengthBars = 4;
    context.normalize();

    const auto structure = planPhraseStructure(context);
    const auto harmony = planHarmony(context, structure);
    const auto graph = buildCompositionKnowledgeGraph(structure, harmony);
    const auto transitions = analyzeCompositionTransitions(graph);

    const auto motifGraph = buildMotifOccurrenceGraph(
        {
            makeMotif(),
            MotifDevelopment::transpose(makeMotif(), 7)
        },
        {0, 8});

    const auto motifProfile = analyzeMotifRecurrence(motifGraph);
    const auto catalog = buildMotifKnowledgeCatalog(motifProfile);
    const auto composition = buildCompositionKnowledgeRecord(
        structure,
        harmony,
        catalog);

    return buildCompositionKnowledgeSnapshot(
        composition,
        graph,
        transitions);
}

void testCanonicalRepresentationIsValid()
{
    const auto snapshot = buildSnapshot();
    const auto representation =
        buildCompositionKnowledgeRepresentation(
            snapshot,
            "phase115-representation");

    expect(
        representation.isValid(),
        "canonical knowledge representation should be valid");

    expect(
        representation.analysisValid,
        "valid representation should be marked analyzed");
}

void testSemanticAndNumericViewsStayAligned()
{
    const auto snapshot = buildSnapshot();
    const auto representation =
        buildCompositionKnowledgeRepresentation(
            snapshot,
            "alignment-test");

    expect(
        representation.sectionCount() ==
            snapshot.sectionCount(),
        "semantic and numeric views must have the same section count");

    expect(
        representation.globalFeatureWidth() ==
            CompositionDatasetSchema::globalFeatureCount,
        "global feature width must come from the existing schema");

    expect(
        representation.sectionFeatureWidth() ==
            CompositionDatasetSchema::sectionFeatureCount,
        "section feature width must come from the existing schema");

    expect(
        representation.datasetSample.sampleId ==
            "alignment-test",
        "sample identity must remain stable");
}

void testRepresentationPreservesExistingSnapshot()
{
    const auto snapshot = buildSnapshot();
    const auto representation =
        buildCompositionKnowledgeRepresentation(
            snapshot,
            "snapshot-preservation");

    expect(
        representation.snapshot.sectionCount() ==
            snapshot.sectionCount(),
        "snapshot section count must be preserved");

    expect(
        representation.snapshot.composition.sectionCount ==
            snapshot.composition.sectionCount,
        "composition knowledge record must be preserved");

    expect(
        representation.snapshot.transitions.risingTransitions ==
            snapshot.transitions.risingTransitions,
        "transition statistics must be preserved");
}

void testNumericProjectionIsFiniteAndBounded()
{
    const auto representation =
        buildCompositionKnowledgeRepresentation(
            buildSnapshot(),
            "numeric-bounds");

    for (const auto value :
         representation.datasetSample.globalFeatures)
    {
        expect(
            std::isfinite(value) &&
            value >= -1.0 &&
            value <= 1.0,
            "global representation values must remain bounded and finite");
    }

    for (const auto& section :
         representation.datasetSample.sectionFeatures)
    {
        for (const auto value : section)
        {
            expect(
                std::isfinite(value) &&
                value >= -1.0 &&
                value <= 1.0,
                "section representation values must remain bounded and finite");
        }
    }
}

void testInvalidSnapshotIsRejected()
{
    CompositionKnowledgeSnapshot snapshot;

    const auto representation =
        buildCompositionKnowledgeRepresentation(
            snapshot,
            "invalid-snapshot");

    expect(
        !representation.analysisValid,
        "invalid snapshot must not create a representation");

    expect(
        !representation.isValid(),
        "invalid snapshot must produce an invalid representation");
}

void testEmptySampleIdIsRejected()
{
    const auto representation =
        buildCompositionKnowledgeRepresentation(
            buildSnapshot(),
            "");

    expect(
        !representation.analysisValid,
        "empty sample id must not create a representation");

    expect(
        !representation.isValid(),
        "empty sample id must produce an invalid representation");
}

void testDefaultRepresentationIsInvalid()
{
    const CompositionKnowledgeRepresentation representation;

    expect(
        !representation.analysisValid,
        "default representation must not be analyzed");

    expect(
        !representation.isValid(),
        "default representation must be invalid");
}

} // namespace

int main()
{
    testCanonicalRepresentationIsValid();
    testSemanticAndNumericViewsStayAligned();
    testRepresentationPreservesExistingSnapshot();
    testNumericProjectionIsFiniteAndBounded();
    testInvalidSnapshotIsRejected();
    testEmptySampleIdIsRejected();
    testDefaultRepresentationIsInvalid();

    std::cout
        << "MIDI-GenGX Composition Knowledge Representation tests passed.\n";

    return 0;
}
