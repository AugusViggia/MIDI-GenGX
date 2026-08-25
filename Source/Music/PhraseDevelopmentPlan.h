#pragma once

#include "../Domain/MusicalContext.h"
#include "PhraseStructure.h"

#include <cstddef>
#include <vector>

namespace midigengx::music
{

enum class PhraseDevelopmentRole
{
    Statement,
    Restatement,
    Development,
    Intensification,
    Response,
    Cadence
};

struct PhraseDevelopmentSection
{
    PhraseDevelopmentRole role =
        PhraseDevelopmentRole::Statement;

    int developmentAmount = 0;
    int motifVariationAmount = 0;
    int transpositionSemitones = 0;
    double timeFactor = 1.0;
    bool invertMotif = false;
    bool retrogradeMotif = false;
    int sequenceRepetitions = 1;
    int sequenceStepSemitones = 0;
};

struct PhraseDevelopmentPlan
{
    std::vector<PhraseDevelopmentSection> sections;

    bool isValid() const noexcept;
};

PhraseDevelopmentPlan planPhraseDevelopment(
    const midigengx::domain::MusicalContext& context,
    const PhraseStructurePlan& structure) noexcept;

} // namespace midigengx::music
