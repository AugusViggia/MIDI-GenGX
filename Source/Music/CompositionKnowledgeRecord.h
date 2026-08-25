#pragma once

#include "HarmonyPlan.h"
#include "MotifKnowledgeCatalog.h"
#include "PhraseStructure.h"

#include <cstddef>
#include <vector>

namespace midigengx::music
{

struct CompositionKnowledgeRecord
{
    double totalLengthBeats = 0.0;
    std::size_t sectionCount = 0;
    std::size_t harmonyEventCount = 0;

    std::vector<PhraseSection> sectionRoles;
    std::vector<int> sectionTensions;

    std::size_t recurringMotifFamilyCount = 0;
    std::size_t totalMotifFamilyCount = 0;
    double averageMotifOccurrences = 0.0;

    double averageSectionTension = 0.0;
    int minimumSectionTension = 0;
    int maximumSectionTension = 0;

    std::size_t majorHarmonyEvents = 0;
    std::size_t minorHarmonyEvents = 0;
    std::size_t diminishedHarmonyEvents = 0;
    std::size_t augmentedHarmonyEvents = 0;
    std::size_t suspendedHarmonyEvents = 0;
    std::size_t unknownHarmonyEvents = 0;

    bool isValid() const noexcept;
};

CompositionKnowledgeRecord buildCompositionKnowledgeRecord(
    const PhraseStructurePlan& structure,
    const HarmonyPlan& harmony,
    const MotifKnowledgeCatalog& motifs) noexcept;

} // namespace midigengx::music
