#include "Music/CompositionKnowledgeSnapshot.h"
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

void buildAll(
    CompositionKnowledgeRecord& composition,
    CompositionKnowledgeGraph& graph,
    CompositionTransitionProfile& transitions)
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

    graph =
        buildCompositionKnowledgeGraph(
            structure,
            harmony);

    transitions =
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

    composition =
        buildCompositionKnowledgeRecord(
            structure,
            harmony,
            catalog);
}

void testSnapshotValidity()
{
    CompositionKnowledgeRecord composition;
    CompositionKnowledgeGraph graph;
    CompositionTransitionProfile transitions;

    buildAll(
        composition,
        graph,
        transitions);

    const auto snapshot =
        buildCompositionKnowledgeSnapshot(
            composition,
            graph,
            transitions);

    expect(
        snapshot.analysisValid,
        "valid inputs mark snapshot analyzed");

    expect(
        snapshot.isValid(),
        "knowledge snapshot is valid");

    expect(
        snapshot.sectionCount() ==
            composition.sectionCount,
        "snapshot preserves section count");
}

void testSectionFeaturesMatchSourceGraph()
{
    CompositionKnowledgeRecord composition;
    CompositionKnowledgeGraph graph;
    CompositionTransitionProfile transitions;

    buildAll(
        composition,
        graph,
        transitions);

    const auto snapshot =
        buildCompositionKnowledgeSnapshot(
            composition,
            graph,
            transitions);

    for (std::size_t index = 0;
         index < snapshot.sections.size();
         ++index)
    {
        const auto& feature =
            snapshot.sections[index];

        expect(
            feature.sectionIndex == index,
            "snapshot section indices are deterministic");

        expect(
            feature.role ==
                graph.sections[index].role,
            "snapshot preserves section role");

        expect(
            feature.tension ==
                graph.sections[index].tension,
            "snapshot preserves section tension");

        expect(
            feature.harmonyScaleDegree ==
                graph.sections[index].harmonyScaleDegree,
            "snapshot preserves harmony degree");
    }
}

void testTransitionFeaturesAreAligned()
{
    CompositionKnowledgeRecord composition;
    CompositionKnowledgeGraph graph;
    CompositionTransitionProfile transitions;

    buildAll(
        composition,
        graph,
        transitions);

    const auto snapshot =
        buildCompositionKnowledgeSnapshot(
            composition,
            graph,
            transitions);

    expect(
        snapshot.sections[0]
            .tensionDeltaFromPrevious == 0,
        "first section has no incoming tension delta");

    for (std::size_t index = 1;
         index < snapshot.sections.size();
         ++index)
    {
        expect(
            snapshot.sections[index]
                .tensionDeltaFromPrevious ==
                graph.transitions[index - 1]
                    .tensionDelta,
            "section feature aligns tension transition");

        expect(
            snapshot.sections[index]
                .harmonicDegreeDeltaFromPrevious ==
                graph.transitions[index - 1]
                    .harmonicDegreeDelta,
            "section feature aligns harmonic transition");
    }
}

void testInvalidComponentRejectsSnapshot()
{
    CompositionKnowledgeRecord composition;
    CompositionKnowledgeGraph graph;
    CompositionTransitionProfile transitions;

    buildAll(
        composition,
        graph,
        transitions);

    transitions.analysisValid = false;

    const auto snapshot =
        buildCompositionKnowledgeSnapshot(
            composition,
            graph,
            transitions);

    expect(
        !snapshot.analysisValid,
        "invalid transition component prevents snapshot analysis");

    expect(
        !snapshot.isValid(),
        "invalid transition component produces invalid snapshot");
}

void testDefaultSnapshotState()
{
    CompositionKnowledgeSnapshot snapshot;

    expect(
        !snapshot.analysisValid,
        "default snapshot is not analyzed");

    expect(
        !snapshot.isValid(),
        "default snapshot is invalid");
}

} // namespace

int main()
{
    testSnapshotValidity();
    testSectionFeaturesMatchSourceGraph();
    testTransitionFeaturesAreAligned();
    testInvalidComponentRejectsSnapshot();
    testDefaultSnapshotState();

    std::cout
        << "MIDI-GenGX Composition Knowledge Snapshot tests passed.\n";

    return 0;
}
