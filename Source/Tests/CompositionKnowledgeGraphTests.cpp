#include "Music/CompositionKnowledgeGraph.h"
#include "Music/MusicalEngine.h"

#include <cstdlib>
#include <iostream>
#include <cmath>

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

void buildPlans(
    PhraseStructurePlan& structure,
    HarmonyPlan& harmony)
{
    midigengx::domain::MusicalContext context;

    context.parameters.lengthBars = 16;
    context.parameters.phraseLengthBars = 4;
    context.normalize();

    structure =
        planPhraseStructure(
            context);

    harmony =
        planHarmony(
            context,
            structure);
}

void testGraphValidityAndTopology()
{
    PhraseStructurePlan structure;
    HarmonyPlan harmony;
    buildPlans(structure, harmony);

    expect(
        structure.isValid() &&
        harmony.isValid(),
        "test inputs form valid structure and harmony plans");

    const auto graph =
        buildCompositionKnowledgeGraph(
            structure,
            harmony);

    expect(
        graph.isValid(),
        "composition knowledge graph is valid");

    expect(
        graph.analysisValid,
        "valid input produces analyzed graph");

    expect(
        graph.sectionCount() ==
            structure.sections.size(),
        "graph preserves section count");

    expect(
        graph.transitionCount() ==
            structure.sections.size() - 1,
        "graph has adjacent section transitions");
}

void testSingleSectionGraph()
{
    PhraseStructurePlan structure;
    HarmonyPlan harmony;

    midigengx::domain::MusicalContext context;
    context.parameters.lengthBars = 1;
    context.parameters.phraseLengthBars = 1;
    context.normalize();

    structure =
        planPhraseStructure(
            context);

    harmony =
        planHarmony(
            context,
            structure);

    expect(
        structure.sections.size() == 1 &&
        harmony.events.size() == 1,
        "single-section test produces one section");

    const auto graph =
        buildCompositionKnowledgeGraph(
            structure,
            harmony);

    expect(
        graph.isValid(),
        "single-section composition graph is valid");

    expect(
        graph.transitionCount() == 0,
        "single-section graph has no transitions");
}

void testSectionKnowledge()
{
    PhraseStructurePlan structure;
    HarmonyPlan harmony;
    buildPlans(structure, harmony);

    const auto graph =
        buildCompositionKnowledgeGraph(
            structure,
            harmony);

    expect(
        graph.sections.front().role ==
            PhraseSection::Opening,
        "first knowledge node preserves opening role");

    expect(
        graph.sections.back().role ==
            PhraseSection::Cadence,
        "last knowledge node preserves cadence role");

    expect(
        graph.sections.front().harmonyQuality !=
            ChordQuality::Unknown,
        "section node preserves inferred harmony quality");

    expect(
        graph.sections.front().tension >= 0 &&
        graph.sections.front().tension <= 100,
        "section tension remains normalized");
}

void testTransitionDeltas()
{
    PhraseStructurePlan structure;
    HarmonyPlan harmony;
    buildPlans(structure, harmony);

    const auto graph =
        buildCompositionKnowledgeGraph(
            structure,
            harmony);

    expect(
        graph.transitions.size() >= 2,
        "graph contains enough transitions for delta testing");

    for (std::size_t index = 0;
         index < graph.transitions.size();
         ++index)
    {
        const auto& edge =
            graph.transitions[index];

        expect(
            edge.sourceSection == index &&
            edge.targetSection == index + 1,
            "transitions preserve sequential topology");

        expect(
            edge.tensionDelta ==
                graph.sections[index + 1].tension -
                graph.sections[index].tension,
            "transition stores exact tension delta");

        expect(
            edge.harmonicDegreeDelta ==
                graph.sections[index + 1].harmonyScaleDegree -
                graph.sections[index].harmonyScaleDegree,
            "transition stores exact harmonic-degree delta");
    }
}

void testInvalidStructureHarmonyMismatch()
{
    PhraseStructurePlan structure;
    HarmonyPlan harmony;
    buildPlans(structure, harmony);

    harmony.events.pop_back();

    const auto graph =
        buildCompositionKnowledgeGraph(
            structure,
            harmony);

    expect(
        !graph.analysisValid,
        "mismatched structure and harmony remain unanalyzed");

    expect(
        !graph.isValid(),
        "invalid composition knowledge graph is rejected");
}

void testEmptyDefaultState()
{
    CompositionKnowledgeGraph graph;

    expect(
        !graph.analysisValid,
        "default composition graph is not analyzed");

    expect(
        !graph.isValid(),
        "default composition graph is invalid");
}

} // namespace

int main()
{
    testGraphValidityAndTopology();
    testSingleSectionGraph();
    testSectionKnowledge();
    testTransitionDeltas();
    testInvalidStructureHarmonyMismatch();
    testEmptyDefaultState();

    std::cout
        << "MIDI-GenGX Composition Knowledge Graph tests passed.\n";

    return 0;
}
