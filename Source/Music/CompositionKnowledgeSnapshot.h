#pragma once

#include "CompositionKnowledgeRecord.h"
#include "CompositionTransitionProfile.h"

#include <cstddef>
#include <vector>

namespace midigengx::music
{

struct CompositionSectionFeature
{
    std::size_t sectionIndex = 0;
    PhraseSection role =
        PhraseSection::Opening;

    int tension = 0;
    int tensionDeltaFromPrevious = 0;

    int harmonyScaleDegree = 0;
    ChordQuality harmonyQuality =
        ChordQuality::Unknown;
    int harmonicDegreeDeltaFromPrevious = 0;

    bool isValid() const noexcept;
};

struct CompositionKnowledgeSnapshot
{
    CompositionKnowledgeRecord composition;
    CompositionTransitionProfile transitions;
    std::vector<CompositionSectionFeature> sections;

    bool analysisValid = false;

    bool isValid() const noexcept;
    std::size_t sectionCount() const noexcept;
};

CompositionKnowledgeSnapshot buildCompositionKnowledgeSnapshot(
    const CompositionKnowledgeRecord& composition,
    const CompositionKnowledgeGraph& graph,
    const CompositionTransitionProfile& transitions) noexcept;

} // namespace midigengx::music
