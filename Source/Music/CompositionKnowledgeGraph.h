#pragma once

#include "CompositionKnowledgeRecord.h"

#include <cstddef>
#include <vector>

namespace midigengx::music
{

struct CompositionSectionKnowledgeNode
{
    std::size_t sectionIndex = 0;
    PhraseSection role =
        PhraseSection::Opening;

    int tension = 0;
    int targetScaleDegree = 0;
    ChordQuality harmonyQuality =
        ChordQuality::Unknown;

    int harmonyScaleDegree = 0;
    int harmonyTension = 0;

    bool isValid() const noexcept;
};

struct CompositionSectionKnowledgeEdge
{
    std::size_t sourceSection = 0;
    std::size_t targetSection = 0;

    int tensionDelta = 0;
    int harmonicDegreeDelta = 0;

    bool isValid() const noexcept;
};

struct CompositionKnowledgeGraph
{
    std::vector<CompositionSectionKnowledgeNode> sections;
    std::vector<CompositionSectionKnowledgeEdge> transitions;

    bool analysisValid = false;

    bool isValid() const noexcept;

    std::size_t sectionCount() const noexcept;
    std::size_t transitionCount() const noexcept;
};

CompositionKnowledgeGraph buildCompositionKnowledgeGraph(
    const PhraseStructurePlan& structure,
    const HarmonyPlan& harmony) noexcept;

} // namespace midigengx::music
