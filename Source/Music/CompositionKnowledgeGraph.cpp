#include "CompositionKnowledgeGraph.h"

#include <algorithm>
#include <cmath>

namespace midigengx::music
{

bool CompositionSectionKnowledgeNode::isValid() const noexcept
{
    return tension >= 0 &&
           tension <= 100 &&
           harmonyScaleDegree >= 0 &&
           harmonyTension >= 0 &&
           harmonyTension <= 100;
}

bool CompositionSectionKnowledgeEdge::isValid() const noexcept
{
    return sourceSection != targetSection;
}

bool CompositionKnowledgeGraph::isValid() const noexcept
{
    if (!analysisValid)
        return false;

    if (sections.empty())
        return transitions.empty();

    for (std::size_t index = 0;
         index < sections.size();
         ++index)
    {
        const auto& section =
            sections[index];

        if (section.sectionIndex != index ||
            !section.isValid())
        {
            return false;
        }
    }

    const std::size_t expectedTransitions =
        sections.size() - 1;

    if (transitions.size() !=
        expectedTransitions)
    {
        return false;
    }

    for (std::size_t index = 0;
         index < transitions.size();
         ++index)
    {
        const auto& edge =
            transitions[index];

        if (!edge.isValid() ||
            edge.sourceSection != index ||
            edge.targetSection != index + 1)
        {
            return false;
        }
    }

    return true;
}

std::size_t CompositionKnowledgeGraph::sectionCount() const noexcept
{
    return sections.size();
}

std::size_t CompositionKnowledgeGraph::transitionCount() const noexcept
{
    return transitions.size();
}

CompositionKnowledgeGraph
buildCompositionKnowledgeGraph(
    const PhraseStructurePlan& structure,
    const HarmonyPlan& harmony) noexcept
{
    CompositionKnowledgeGraph graph;

    if (!structure.isValid() ||
        !harmony.isValid() ||
        structure.sections.size() !=
            harmony.events.size() ||
        std::abs(
            structure.totalLengthBeats -
            harmony.totalLengthBeats) >
            1.0e-9)
    {
        return graph;
    }

    graph.analysisValid = true;

    graph.sections.reserve(
        structure.sections.size());

    for (std::size_t index = 0;
         index < structure.sections.size();
         ++index)
    {
        const auto& sourceSection =
            structure.sections[index];

        const auto& harmonyEvent =
            harmony.events[index];

        CompositionSectionKnowledgeNode node;

        node.sectionIndex = index;
        node.role =
            sourceSection.section;
        node.tension =
            sourceSection.tension;
        node.targetScaleDegree =
            sourceSection.targetScaleDegree;
        node.harmonyQuality =
            harmonyEvent.quality;
        node.harmonyScaleDegree =
            harmonyEvent.scaleDegree;
        node.harmonyTension =
            harmonyEvent.tension;

        graph.sections.push_back(node);
    }

    if (graph.sections.size() > 1)
    {
        graph.transitions.reserve(
            graph.sections.size() - 1);

        for (std::size_t index = 0;
             index + 1 <
                 graph.sections.size();
             ++index)
        {
            const auto& current =
                graph.sections[index];

            const auto& next =
                graph.sections[index + 1];

            CompositionSectionKnowledgeEdge edge;

            edge.sourceSection =
                index;
            edge.targetSection =
                index + 1;

            edge.tensionDelta =
                next.tension -
                current.tension;

            edge.harmonicDegreeDelta =
                next.harmonyScaleDegree -
                current.harmonyScaleDegree;

            graph.transitions.push_back(edge);
        }
    }

    return graph;
}

} // namespace midigengx::music
