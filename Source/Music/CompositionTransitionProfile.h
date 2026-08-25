#pragma once

#include "CompositionKnowledgeGraph.h"

#include <cstddef>
#include <vector>

namespace midigengx::music
{

enum class TensionTransitionKind
{
    Flat,
    Rising,
    Falling
};

struct CompositionTransitionProfile
{
    std::vector<TensionTransitionKind> tensionTransitions;
    bool analysisValid = false;
    std::size_t sectionCount = 0;
    std::vector<int> tensionDeltas;
    std::vector<int> harmonicDegreeDeltas;

    std::size_t risingTransitions = 0;
    std::size_t fallingTransitions = 0;
    std::size_t flatTransitions = 0;

    int strongestRise = 0;
    int strongestFall = 0;

    std::size_t peakSectionIndex = 0;
    int peakTension = 0;

    bool isValid() const noexcept;
};

CompositionTransitionProfile analyzeCompositionTransitions(
    const CompositionKnowledgeGraph& graph) noexcept;

} // namespace midigengx::music
