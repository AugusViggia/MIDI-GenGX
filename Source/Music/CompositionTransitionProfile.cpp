#include "CompositionTransitionProfile.h"

#include <algorithm>

namespace midigengx::music
{

bool CompositionTransitionProfile::isValid() const noexcept
{
    if (!analysisValid)
        return false;

    if (tensionTransitions.size() !=
            tensionDeltas.size() ||
        tensionTransitions.size() !=
            harmonicDegreeDeltas.size())
    {
        return false;
    }

    const auto transitionCount =
        tensionTransitions.size();

    if (risingTransitions +
            fallingTransitions +
            flatTransitions !=
        transitionCount)
    {
        return false;
    }

    if (strongestRise < 0 ||
        strongestFall > 0 ||
        peakTension < 0 ||
        peakTension > 100)
    {
        return false;
    }

    if (transitionCount == 0)
    {
        if (sectionCount == 0)
        {
            return peakSectionIndex == 0 &&
                   peakTension == 0;
        }

        return sectionCount == 1 &&
               peakSectionIndex == 0;
    }

    for (std::size_t i = 0;
         i < transitionCount;
         ++i)
    {
        const auto delta =
            tensionDeltas[i];

        const auto expectedKind =
            delta > 0
                ? TensionTransitionKind::Rising
                : (delta < 0
                    ? TensionTransitionKind::Falling
                    : TensionTransitionKind::Flat);

        if (tensionTransitions[i] !=
            expectedKind)
        {
            return false;
        }
    }

    return true;
}

CompositionTransitionProfile
analyzeCompositionTransitions(
    const CompositionKnowledgeGraph& graph) noexcept
{
    CompositionTransitionProfile profile;

    if (!graph.isValid())
        return profile;

    profile.analysisValid = true;
    profile.sectionCount =
        graph.sections.size();

    profile.tensionTransitions.reserve(
        graph.transitions.size());

    profile.tensionDeltas.reserve(
        graph.transitions.size());

    profile.harmonicDegreeDeltas.reserve(
        graph.transitions.size());

    for (const auto& edge :
         graph.transitions)
    {
        profile.tensionDeltas.push_back(
            edge.tensionDelta);

        profile.harmonicDegreeDeltas.push_back(
            edge.harmonicDegreeDelta);

        if (edge.tensionDelta > 0)
        {
            profile.tensionTransitions.push_back(
                TensionTransitionKind::Rising);

            ++profile.risingTransitions;

            profile.strongestRise =
                std::max(
                    profile.strongestRise,
                    edge.tensionDelta);
        }
        else if (edge.tensionDelta < 0)
        {
            profile.tensionTransitions.push_back(
                TensionTransitionKind::Falling);

            ++profile.fallingTransitions;

            profile.strongestFall =
                std::min(
                    profile.strongestFall,
                    edge.tensionDelta);
        }
        else
        {
            profile.tensionTransitions.push_back(
                TensionTransitionKind::Flat);

            ++profile.flatTransitions;
        }
    }

    if (!graph.sections.empty())
    {
        profile.peakSectionIndex = 0;
        profile.peakTension =
            graph.sections.front().tension;

        for (std::size_t index = 1;
             index < graph.sections.size();
             ++index)
        {
            if (graph.sections[index].tension >
                profile.peakTension)
            {
                profile.peakTension =
                    graph.sections[index].tension;
                profile.peakSectionIndex =
                    index;
            }
        }
    }

    return profile;
}

} // namespace midigengx::music
