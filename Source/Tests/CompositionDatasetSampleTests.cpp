#include "Music/CompositionDatasetSample.h"
#include "Music/MotifDevelopment.h"

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

    const auto structure =
        planPhraseStructure(context);

    const auto harmony =
        planHarmony(
            context,
            structure);

    const auto graph =
        buildCompositionKnowledgeGraph(
            structure,
            harmony);

    const auto transitions =
        analyzeCompositionTransitions(
            graph);

    const auto motifGraph =
        buildMotifOccurrenceGraph(
            {
                makeMotif(),
                MotifDevelopment::transpose(
                    makeMotif(),
                    7)
            },
            {0, 8});

    const auto motifProfile =
        analyzeMotifRecurrence(
            motifGraph);

    const auto catalog =
        buildMotifKnowledgeCatalog(
            motifProfile);

    const auto composition =
        buildCompositionKnowledgeRecord(
            structure,
            harmony,
            catalog);

    return buildCompositionKnowledgeSnapshot(
        composition,
        graph,
        transitions);
}

void testSchemaShape()
{
    const auto snapshot =
        buildSnapshot();

    const auto sample =
        buildCompositionDatasetSample(
            snapshot,
            "phase32-test");

    expect(
        sample.isValid(),
        "dataset sample is valid");

    expect(
        CompositionDatasetSample::schemaVersion == 1,
        "dataset schema version is one");

    expect(
        sample.globalFeatures.size() == 13,
        "global feature vector has stable width");

    expect(
        sample.sectionFeatures.size() ==
            snapshot.sectionCount(),
        "section feature rows match section count");

    for (const auto& row :
         sample.sectionFeatures)
    {
        expect(
            row.size() == 6,
            "section feature vector has stable width");
    }
}

void testFeaturesAreNormalized()
{
    const auto snapshot =
        buildSnapshot();

    const auto sample =
        buildCompositionDatasetSample(
            snapshot,
            "normalized-test");

    for (const auto value :
         sample.globalFeatures)
    {
        expect(
            value >= -1.0 &&
            value <= 1.0,
            "global features remain in normalized range");
    }

    for (const auto& row :
         sample.sectionFeatures)
    {
        for (const auto value :
             row)
        {
            expect(
                value >= -1.0 &&
                value <= 1.0,
                "section features remain in normalized range");
        }
    }
}

void testDeterministicFeatureOrder()
{
    const auto snapshot =
        buildSnapshot();

    const auto first =
        buildCompositionDatasetSample(
            snapshot,
            "deterministic");

    const auto second =
        buildCompositionDatasetSample(
            snapshot,
            "deterministic");

    expect(
        first.globalFeatures ==
            second.globalFeatures,
        "global feature encoding is deterministic");

    expect(
        first.sectionFeatures ==
            second.sectionFeatures,
        "section feature encoding is deterministic");
}

void testFirstSectionHasZeroIncomingDelta()
{
    const auto snapshot =
        buildSnapshot();

    const auto sample =
        buildCompositionDatasetSample(
            snapshot,
            "delta-test");

    expect(
        sample.sectionFeatures.front()[2] == 0.0,
        "first section has zero incoming tension delta");

    expect(
        sample.sectionFeatures.front()[5] == 0.0,
        "first section has zero incoming harmonic delta");
}

void testInvalidSnapshotProducesInvalidSample()
{
    CompositionKnowledgeSnapshot snapshot;

    const auto sample =
        buildCompositionDatasetSample(
            snapshot,
            "invalid-test");

    expect(
        !sample.analysisValid,
        "invalid snapshot remains unanalyzed");

    expect(
        !sample.isValid(),
        "invalid snapshot produces invalid dataset sample");
}

void testEmptySampleIdIsRejected()
{
    const auto snapshot =
        buildSnapshot();

    const auto sample =
        buildCompositionDatasetSample(
            snapshot,
            "");

    expect(
        !sample.analysisValid,
        "empty sample id prevents dataset sample creation");

    expect(
        !sample.isValid(),
        "empty sample id produces invalid dataset sample");
}

} // namespace

int main()
{
    testSchemaShape();
    testFeaturesAreNormalized();
    testDeterministicFeatureOrder();
    testFirstSectionHasZeroIncomingDelta();
    testInvalidSnapshotProducesInvalidSample();
    testEmptySampleIdIsRejected();

    std::cout
        << "MIDI-GenGX Composition Dataset Sample tests passed.\n";

    return 0;
}
