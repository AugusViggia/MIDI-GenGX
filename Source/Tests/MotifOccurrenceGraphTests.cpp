#include "Music/MotifOccurrenceGraph.h"
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
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}

Motif makeMotif(
    int anchor = 0)
{
    Motif motif;
    motif.lengthBeats = 2.0;
    motif.notes =
    {
        MotifNote{0.0, 0.5, anchor + 0, 0},
        MotifNote{0.5, 0.5, anchor + 2, 1},
        MotifNote{1.0, 0.5, anchor + 5, -1},
        MotifNote{1.5, 0.5, anchor + 3, 0}
    };
    return motif;
}

void testGraphConstructionState()
{
    MotifOccurrenceGraph unconstructed;

    expect(
        !unconstructed.isValid(),
        "default graph is not considered constructed");

    const auto emptyGraph =
        buildMotifOccurrenceGraph({});

    expect(
        emptyGraph.analysisValid,
        "builder marks empty graph as constructed");

    expect(
        emptyGraph.isValid(),
        "builder-produced empty graph is valid");
}

void testOccurrenceNodes()
{
    const auto repeated =
        makeMotif();

    const auto transposed =
        MotifDevelopment::transpose(
            repeated,
            7);

    const auto unrelated =
        makeMotif();

    const auto graph =
        buildMotifOccurrenceGraph(
            {repeated, transposed, unrelated},
            {4, 7, 9});

    expect(
        graph.isValid(),
        "motif occurrence graph is valid");

    expect(
        graph.occurrences.size() == 3,
        "graph preserves all valid motif occurrences");

    expect(
        graph.countOccurrencesOf(
            fingerprintMotif(repeated)) == 3,
        "canonical identity groups transposed and repeated motifs");
}

void testRelationshipEdges()
{
    const auto source =
        makeMotif();

    const auto transposed =
        MotifDevelopment::transpose(
            source,
            5);

    const auto changed =
        MotifDevelopment::transposeAndStretch(
            source,
            5,
            0.5);

    const auto graph =
        buildMotifOccurrenceGraph(
            {source, transposed, changed});

    bool foundTransposition = false;
    bool foundVariation = false;

    for (const auto& edge :
         graph.edges)
    {
        if (edge.relationship.kind ==
            MotifRelationshipKind::Transposition)
        {
            foundTransposition = true;

            expect(
                edge.relationship.transpositionSemitones == 5,
                "graph preserves transposition displacement");
        }

        if (edge.relationship.kind ==
            MotifRelationshipKind::CompoundVariation ||
            edge.relationship.kind ==
            MotifRelationshipKind::RhythmicVariation)
        {
            foundVariation = true;
        }
    }

    expect(
        foundTransposition,
        "graph contains transposition relationship");

    expect(
        foundVariation,
        "graph contains transformed motif relationship");
}

void testInvalidMotifsAreExcluded()
{
    Motif invalid;

    const auto graph =
        buildMotifOccurrenceGraph(
            {makeMotif(), invalid});

    expect(
        graph.isValid(),
        "graph with invalid source is still valid");

    expect(
        graph.occurrences.size() == 1,
        "invalid motifs are excluded from occurrence graph");
}

void testSparseSourcePhraseMetadata()
{
    const auto graph =
        buildMotifOccurrenceGraph(
            {makeMotif(), makeMotif(7)},
            {12});

    expect(
        graph.occurrences.size() == 2,
        "sparse phrase metadata does not drop valid occurrences");

    expect(
        graph.occurrences[0].sourcePhraseIndex == 12 &&
        graph.occurrences[1].sourcePhraseIndex == 1,
        "occurrence metadata falls back deterministically");
}

} // namespace

int main()
{
    testGraphConstructionState();
    testOccurrenceNodes();
    testRelationshipEdges();
    testInvalidMotifsAreExcluded();
    testSparseSourcePhraseMetadata();

    std::cout
        << "MIDI-GenGX Motif Occurrence Graph tests passed.\n";

    return 0;
}
