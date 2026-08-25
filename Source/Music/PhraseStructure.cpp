#include "PhraseStructure.h"
#include "PhraseTensionArc.h"

#include <algorithm>

namespace midigengx::music
{

int cadenceTargetScaleDegree(
    midigengx::domain::CadenceStyle style,
    int scaleSize) noexcept
{
    if (scaleSize <= 0)
        return 0;

    switch (style)
    {
        case midigengx::domain::CadenceStyle::Root:
            return 0;

        case midigengx::domain::CadenceStyle::Fifth:
            return std::min(4, scaleSize - 1);

        case midigengx::domain::CadenceStyle::Third:
            return std::min(2, scaleSize - 1);

        case midigengx::domain::CadenceStyle::Open:
            return std::min(
                scaleSize - 1,
                std::max(1, scaleSize / 2));

        case midigengx::domain::CadenceStyle::Custom:
            return 0;
    }

    return 0;
}

bool PhraseStructurePlan::isValid() const noexcept
{
    if (totalLengthBeats <= 0.0 ||
        sections.empty())
    {
        return false;
    }

    double previousEnd = 0.0;

    for (const auto& section : sections)
    {
        if (section.startBeat < 0.0 ||
            section.endBeat <= section.startBeat ||
            section.endBeat > totalLengthBeats + 1.0e-9)
        {
            return false;
        }

        if (section.startBeat + 1.0e-9 < previousEnd)
            return false;

        if (section.targetScaleDegree < 0 ||
            section.tension < 0 ||
            section.tension > 100)
        {
            return false;
        }

        previousEnd = section.endBeat;
    }

    return true;
}

PhraseStructurePlan planPhraseStructure(
    const midigengx::domain::MusicalContext& inputContext) noexcept
{
    auto context = inputContext;
    context.normalize();

    const int phraseBars =
        std::max(1, context.parameters.phraseLengthBars);

    const int totalBars =
        std::max(
            phraseBars,
            context.parameters.lengthBars);

    const double phraseLength =
        static_cast<double>(phraseBars) * 4.0;

    const int phraseCount =
        std::max(
            1,
            (totalBars + phraseBars - 1) /
                phraseBars);

    PhraseStructurePlan plan;
    plan.totalLengthBeats =
        static_cast<double>(totalBars) * 4.0;

    const int scaleSize = 7;

    for (int index = 0;
         index < phraseCount;
         ++index)
    {
        PhraseSectionPlan section;

        section.startBeat =
            static_cast<double>(index) *
            phraseLength;

        section.endBeat =
            std::min(
                plan.totalLengthBeats,
                section.startBeat + phraseLength);

        if (index == 0)
        {
            section.section =
                PhraseSection::Opening;
        }
        else if (index == phraseCount - 1)
        {
            section.section =
                PhraseSection::Cadence;
            section.targetScaleDegree =
                cadenceTargetScaleDegree(
                    context.parameters.cadenceStyle,
                    scaleSize);
        }
        else if (index == phraseCount - 2)
        {
            section.section =
                PhraseSection::Preparation;
        }
        else
        {
            section.section =
                PhraseSection::Development;
        }

        section.tension =
            PhraseTensionArc::sectionTension(
                section.startBeat,
                section.endBeat,
                plan.totalLengthBeats,
                context.parameters.phraseContour,
                context.parameters.tension,
                context.parameters.cadenceStrength,
                section.section == PhraseSection::Cadence);

        plan.sections.push_back(section);
    }

    return plan;
}

} // namespace midigengx::music
