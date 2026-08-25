#include "PhraseDevelopmentPlan.h"

#include <algorithm>
#include <cmath>

namespace midigengx::music
{
namespace
{

PhraseDevelopmentRole resolveDevelopmentRole(
    std::size_t index,
    std::size_t sectionCount,
    int repetition) noexcept
{
    if (sectionCount == 0)
        return PhraseDevelopmentRole::Statement;

    if (index == 0)
        return PhraseDevelopmentRole::Statement;

    if (index + 1 == sectionCount)
        return PhraseDevelopmentRole::Cadence;

    if (sectionCount >= 2 &&
        index + 1 == sectionCount - 1)
    {
        return PhraseDevelopmentRole::Intensification;
    }

    if (repetition >= 75)
        return PhraseDevelopmentRole::Restatement;

    if (sectionCount >= 5 &&
        index == sectionCount - 3)
    {
        return PhraseDevelopmentRole::Response;
    }

    return PhraseDevelopmentRole::Development;
}

} // namespace

bool PhraseDevelopmentPlan::isValid() const noexcept
{
    for (const auto& section : sections)
    {
        if (section.developmentAmount < 0 ||
            section.developmentAmount > 100 ||
            !std::isfinite(section.timeFactor) ||
            section.timeFactor <= 0.0)
        {
            return false;
        }

        if (std::abs(section.transpositionSemitones) > 24)
            return false;

        if (section.sequenceRepetitions < 1 ||
            section.sequenceRepetitions > 8 ||
            std::abs(section.sequenceStepSemitones) > 12)
        {
            return false;
        }
    }

    return true;
}

PhraseDevelopmentPlan planPhraseDevelopment(
    const midigengx::domain::MusicalContext& inputContext,
    const PhraseStructurePlan& structure) noexcept
{
    auto context = inputContext;
    context.normalize();

    PhraseDevelopmentPlan plan;
    plan.sections.reserve(
        structure.sections.size());

    const int variation =
        std::clamp(
            context.parameters.variation,
            0,
            100);

    const int repetition =
        std::clamp(
            context.parameters.repetition,
            0,
            100);

    for (std::size_t index = 0;
         index < structure.sections.size();
         ++index)
    {
        PhraseDevelopmentSection section;


        section.role =
            resolveDevelopmentRole(
                index,
                structure.sections.size(),
                repetition);

        switch (section.role)
        {
            case PhraseDevelopmentRole::Statement:
                section.developmentAmount = 0;
                break;

            case PhraseDevelopmentRole::Cadence:
                section.developmentAmount =
                    std::min(
                        100,
                        std::max(
                            variation,
                            context.parameters.tension));
                break;

            case PhraseDevelopmentRole::Intensification:
                section.developmentAmount =
                    std::clamp(
                        std::max(
                            variation,
                            context.parameters.tension + 15),
                        0,
                        100);
                break;

            case PhraseDevelopmentRole::Restatement:
                section.developmentAmount =
                    std::clamp(
                        100 - repetition,
                        0,
                        35);
                break;

            case PhraseDevelopmentRole::Response:
                section.developmentAmount =
                    std::clamp(
                        std::max(
                            variation,
                            50),
                        0,
                        100);
                break;

            case PhraseDevelopmentRole::Development:
                section.developmentAmount =
                    std::clamp(
                        std::max(
                            variation,
                            100 - repetition),
                        0,
                        100);
                break;
        }

        const int amount =
            section.developmentAmount;

        section.motifVariationAmount =
            std::clamp(
                context.parameters.variation *
                (section.role ==
                         PhraseDevelopmentRole::Restatement
                     ? 0
                     : (section.role ==
                                PhraseDevelopmentRole::Statement
                            ? 0
                            : (section.role ==
                                       PhraseDevelopmentRole::Cadence
                                   ? 20
                                   : 75))),
                0,
                100);

        switch (section.role)
        {
            case PhraseDevelopmentRole::Statement:
            case PhraseDevelopmentRole::Restatement:
                section.transpositionSemitones = 0;
                section.timeFactor = 1.0;
                section.invertMotif = false;
                section.retrogradeMotif = false;
                section.sequenceRepetitions = 1;
                section.sequenceStepSemitones = 0;
                break;

            case PhraseDevelopmentRole::Development:
                section.transpositionSemitones =
                    amount / 25;
                section.transpositionSemitones =
                    index % 2 == 0
                        ? section.transpositionSemitones
                        : -section.transpositionSemitones;

                section.timeFactor =
                    amount >= 75
                        ? 0.5
                        : (amount >= 45 ? 0.75 : 1.0);

                section.invertMotif =
                    amount >= 70 &&
                    index % 2 == 1;

                section.retrogradeMotif = false;

                section.sequenceRepetitions =
                    amount >= 75 ? 2 : 1;

                section.sequenceStepSemitones =
                    amount >= 75
                        ? (index % 2 == 0 ? 2 : -2)
                        : 0;
                break;

            case PhraseDevelopmentRole::Intensification:
                section.transpositionSemitones =
                    std::max(
                        1,
                        amount / 30);

                section.timeFactor =
                    amount >= 70
                        ? 0.5
                        : 0.75;

                section.invertMotif =
                    amount >= 80;

                section.retrogradeMotif = false;

                section.sequenceRepetitions =
                    amount >= 85 ? 2 : 1;

                section.sequenceStepSemitones =
                    amount >= 85 ? 1 : 0;
                break;

            case PhraseDevelopmentRole::Response:
                section.transpositionSemitones =
                    -std::max(
                        1,
                        amount / 35);

                section.timeFactor =
                    amount >= 70
                        ? 0.75
                        : 1.0;

                section.invertMotif =
                    false;

                section.retrogradeMotif =
                    amount >= 75;

                section.sequenceRepetitions =
                    amount >= 80 ? 2 : 1;

                section.sequenceStepSemitones =
                    amount >= 80 ? -1 : 0;
                break;

            case PhraseDevelopmentRole::Cadence:
            {
                // A two-phrase form has no separate Development/Response
                // section before the cadence. Keep the cadence structurally
                // stable, but allow Variation/Repetition to influence its
                // melodic statement so those controls retain their contract
                // even for short (A -> Cadence) forms.
                const int cadenceDevelopment =
                    std::clamp(
                        amount,
                        0,
                        100);

                section.transpositionSemitones =
                    cadenceDevelopment >= 50
                        ? std::clamp(
                              context.parameters.repetition >= 75
                                  ? 0
                                  : (cadenceDevelopment / 35),
                              0,
                              2)
                        : 0;

                section.timeFactor = 1.0;
                section.invertMotif = false;
                section.retrogradeMotif = false;
                section.sequenceRepetitions = 1;
                section.sequenceStepSemitones = 0;
                break;
            }
        }

        plan.sections.push_back(section);
    }

    return plan;
}

} // namespace midigengx::music
