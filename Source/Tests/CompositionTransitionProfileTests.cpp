#include "Music/CompositionTransitionProfile.h"

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

CompositionKnowledgeGraph makeGraph(
    std::initializer_list<int> tensions)
{
    CompositionKnowledgeGraph graph;
    graph.analysisValid = true;

    std::size_t index = 0;

    for (const auto tension :
         tensions)
    {
        CompositionSectionKnowledgeNode node;
        node.sectionIndex = index;
        node.tension = tension;
        node.harmonyScaleDegree =
            static_cast<int>(index);

        graph.sections.push_back(node);
        ++index;
    }

    for (std::size_t i = 0;
         i + 1 < graph.sections.size();
         ++i)
    {
        CompositionSectionKnowledgeEdge edge;
        edge.sourceSection = i;
        edge.targetSection = i + 1;
        edge.tensionDelta =
            graph.sections[i + 1].tension -
            graph.sections[i].tension;

        edge.harmonicDegreeDelta =
            graph.sections[i + 1].harmonyScaleDegree -
            graph.sections[i].harmonyScaleDegree;

        graph.transitions.push_back(edge);
    }

    return graph;
}

void testRisingAndFallingTransitions()
{
    const auto graph =
        makeGraph({20, 35, 55, 40});

    const auto profile =
        analyzeCompositionTransitions(
            graph);

    expect(
        profile.analysisValid,
        "valid graph marks transition analysis complete");

    expect(
        profile.isValid(),
        "transition profile is valid");

    expect(
        profile.risingTransitions == 2 &&
        profile.fallingTransitions == 1 &&
        profile.flatTransitions == 0,
        "transition directions are counted");

    expect(
        profile.strongestRise == 20,
        "strongest rise is detected");

    expect(
        profile.strongestFall == -15,
        "strongest fall is detected");
}

void testFlatTransition()
{
    const auto graph =
        makeGraph({50, 50, 65});

    const auto profile =
        analyzeCompositionTransitions(
            graph);

    expect(
        profile.analysisValid,
        "flat transition analysis completes");

    expect(
        profile.isValid(),
        "flat transition profile is valid");

    expect(
        profile.flatTransitions == 1,
        "flat transition is counted");

    expect(
        profile.tensionTransitions[0] ==
            TensionTransitionKind::Flat,
        "flat transition receives flat classification");
}

void testPeakDetection()
{
    const auto graph =
        makeGraph({25, 60, 85, 70});

    const auto profile =
        analyzeCompositionTransitions(
            graph);

    expect(
        profile.peakSectionIndex == 2,
        "highest tension section is detected");

    expect(
        profile.peakTension == 85,
        "peak tension value is preserved");
}

void testHarmonicTransitionPreservation()
{
    auto graph =
        makeGraph({30, 45, 40});

    graph.transitions[0].harmonicDegreeDelta = 4;
    graph.transitions[1].harmonicDegreeDelta = -2;

    const auto profile =
        analyzeCompositionTransitions(
            graph);

    expect(
        profile.harmonicDegreeDeltas ==
            std::vector<int>({4, -2}),
        "harmonic transition deltas are preserved");
}

void testInvalidGraphProducesInvalidProfile()
{
    CompositionKnowledgeGraph graph;

    const auto profile =
        analyzeCompositionTransitions(
            graph);

    expect(
        !profile.isValid(),
        "invalid graph produces invalid transition profile");
}

void testEmptyAnalyzedGraph()
{
    CompositionKnowledgeGraph graph;
    graph.analysisValid = true;

    const auto profile =
        analyzeCompositionTransitions(
            graph);

    expect(
        profile.analysisValid,
        "empty analyzed graph completes transition analysis");

    expect(
        profile.sectionCount == 0,
        "empty analyzed graph records zero sections");

    expect(
        profile.isValid(),
        "empty analyzed graph produces valid empty profile");
}

void testDefaultProfileState()
{
    CompositionTransitionProfile profile;

    expect(
        !profile.analysisValid,
        "default transition profile is not analyzed");

    expect(
        !profile.isValid(),
        "default transition profile is invalid");
}

void testSingleSectionComposition()
{
    const auto graph =
        makeGraph({50});

    const auto profile =
        analyzeCompositionTransitions(
            graph);

    expect(
        profile.analysisValid,
        "single-section transition analysis completes");

    expect(
        profile.sectionCount == 1,
        "single-section transition profile records section count");

    expect(
        profile.isValid(),
        "single-section transition profile is valid");

    expect(
        profile.tensionDeltas.empty() &&
        profile.risingTransitions == 0 &&
        profile.fallingTransitions == 0 &&
        profile.flatTransitions == 0,
        "single section has no transitions");

    expect(
        profile.peakSectionIndex == 0 &&
        profile.peakTension == 50,
        "single section is its own peak");
}

} // namespace

int main()
{
    testRisingAndFallingTransitions();
    testFlatTransition();
    testPeakDetection();
    testHarmonicTransitionPreservation();
    testInvalidGraphProducesInvalidProfile();
    testEmptyAnalyzedGraph();
    testDefaultProfileState();
    testSingleSectionComposition();

    std::cout
        << "MIDI-GenGX Composition Transition Profile tests passed.\n";

    return 0;
}
