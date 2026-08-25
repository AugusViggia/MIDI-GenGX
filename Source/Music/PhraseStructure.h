#pragma once

#include "../Domain/MusicalContext.h"

#include <cstddef>
#include <vector>

namespace midigengx::music
{

enum class PhraseSection
{
    Opening,
    Development,
    Preparation,
    Cadence
};

struct PhraseSectionPlan
{
    PhraseSection section = PhraseSection::Opening;
    double startBeat = 0.0;
    double endBeat = 0.0;
    int targetScaleDegree = 0;
    int tension = 50;
};

struct PhraseStructurePlan
{
    double totalLengthBeats = 0.0;
    std::vector<PhraseSectionPlan> sections;

    bool isValid() const noexcept;
};

PhraseStructurePlan planPhraseStructure(
    const midigengx::domain::MusicalContext& context) noexcept;

int cadenceTargetScaleDegree(
    midigengx::domain::CadenceStyle style,
    int scaleSize) noexcept;

} // namespace midigengx::music
