#include "CompositionKnowledgeSnapshot.h"

namespace midigengx::music
{

bool CompositionSectionFeature::isValid() const noexcept
{
    return tension >= 0 &&
           tension <= 100 &&
           tensionDeltaFromPrevious >= -100 &&
           tensionDeltaFromPrevious <= 100 &&
           harmonyScaleDegree >= 0 &&
           harmonicDegreeDeltaFromPrevious >= -128 &&
           harmonicDegreeDeltaFromPrevious <= 128;
}

bool CompositionKnowledgeSnapshot::isValid() const noexcept
{
    if (!analysisValid ||
        !composition.isValid() ||
        !transitions.isValid() ||
        sections.size() !=
            composition.sectionCount)
    {
        return false;
    }

    for (std::size_t index = 0;
         index < sections.size();
         ++index)
    {
        if (!sections[index].isValid() ||
            sections[index].sectionIndex != index)
        {
            return false;
        }

        if (sections[index].tension !=
            composition.sectionTensions[index] ||
            sections[index].role !=
            composition.sectionRoles[index])
        {
            return false;
        }
    }

    return true;
}

std::size_t CompositionKnowledgeSnapshot::sectionCount() const noexcept
{
    return sections.size();
}

CompositionKnowledgeSnapshot
buildCompositionKnowledgeSnapshot(
    const CompositionKnowledgeRecord& composition,
    const CompositionKnowledgeGraph& graph,
    const CompositionTransitionProfile& transitions) noexcept
{
    CompositionKnowledgeSnapshot snapshot;

    if (!composition.isValid() ||
        !graph.isValid() ||
        !transitions.isValid() ||
        graph.sectionCount() !=
            composition.sectionCount ||
        transitions.sectionCount !=
            composition.sectionCount)
    {
        return snapshot;
    }

    snapshot.analysisValid = true;
    snapshot.composition = composition;
    snapshot.transitions = transitions;

    snapshot.sections.reserve(
        graph.sections.size());

    for (std::size_t index = 0;
         index < graph.sections.size();
         ++index)
    {
        const auto& node =
            graph.sections[index];

        CompositionSectionFeature feature;

        feature.sectionIndex =
            index;
        feature.role =
            node.role;
        feature.tension =
            node.tension;
        feature.harmonyScaleDegree =
            node.harmonyScaleDegree;
        feature.harmonyQuality =
            node.harmonyQuality;

        if (index > 0)
        {
            const auto& edge =
                graph.transitions[index - 1];

            feature.tensionDeltaFromPrevious =
                edge.tensionDelta;

            feature.harmonicDegreeDeltaFromPrevious =
                edge.harmonicDegreeDelta;
        }

        snapshot.sections.push_back(
            feature);
    }

    return snapshot;
}

} // namespace midigengx::music
