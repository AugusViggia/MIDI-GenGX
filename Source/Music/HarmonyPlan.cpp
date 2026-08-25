#include "HarmonyPlan.h"

#include <algorithm>

namespace midigengx::music
{
namespace
{

int normalizePitchClass(int value) noexcept
{
    const int result = value % 12;
    return result < 0 ? result + 12 : result;
}

int intervalClass(
    int fromPitchClass,
    int toPitchClass) noexcept
{
    return normalizePitchClass(
        toPitchClass - fromPitchClass);
}

ChordQuality qualityFromIntervals(
    int third,
    int fifth) noexcept
{
    if (third == 4 && fifth == 7)
        return ChordQuality::Major;

    if (third == 3 && fifth == 7)
        return ChordQuality::Minor;

    if (third == 3 && fifth == 6)
        return ChordQuality::Diminished;

    if (third == 4 && fifth == 8)
        return ChordQuality::Augmented;

    if (third == 5 && fifth == 7)
        return ChordQuality::Suspended;

    return ChordQuality::Unknown;
}

} // namespace

bool HarmonyEvent::isValid() const noexcept
{
    return startBeat >= 0.0 &&
           endBeat > startBeat &&
           rootPitchClass >= 0 &&
           rootPitchClass < 12 &&
           scaleDegree >= 0 &&
           tension >= 0 &&
           tension <= 100;
}

bool HarmonyPlan::isValid() const noexcept
{
    if (totalLengthBeats <= 0.0 ||
        events.empty())
    {
        return false;
    }

    double previousEnd = 0.0;

    for (const auto& event : events)
    {
        if (!event.isValid())
            return false;

        if (event.endBeat > totalLengthBeats + 1.0e-9)
            return false;

        if (event.startBeat + 1.0e-9 < previousEnd)
            return false;

        previousEnd = event.endBeat;
    }

    return true;
}

ChordQuality inferTriadQuality(
    const std::vector<int>& scalePitchClasses,
    int scaleDegree) noexcept
{
    if (scalePitchClasses.size() < 3 ||
        scaleDegree < 0 ||
        scaleDegree >= static_cast<int>(
            scalePitchClasses.size()))
    {
        return ChordQuality::Unknown;
    }

    const int scaleSize =
        static_cast<int>(scalePitchClasses.size());

    const int root =
        scalePitchClasses[
            static_cast<std::size_t>(scaleDegree)];

    const int third =
        scalePitchClasses[
            static_cast<std::size_t>(
                (scaleDegree + 2) % scaleSize)];

    const int fifth =
        scalePitchClasses[
            static_cast<std::size_t>(
                (scaleDegree + 4) % scaleSize)];

    return qualityFromIntervals(
        intervalClass(root, third),
        intervalClass(root, fifth));
}

HarmonyPlan planHarmony(
    const midigengx::domain::MusicalContext& inputContext,
    const PhraseStructurePlan& structure) noexcept
{
    auto context = inputContext;
    context.normalize();

    HarmonyPlan plan;
    plan.totalLengthBeats =
        structure.totalLengthBeats;

    if (!structure.isValid())
        return plan;

    const int rootPitchClass =
        midigengx::domain::toPitchClass(context.key);

    const auto pitchClasses =
        context.scale.getPitchClasses(rootPitchClass);

    if (pitchClasses.size() < 3)
        return plan;

    const int scaleSize =
        static_cast<int>(pitchClasses.size());

    const int preparationDegree =
        scaleSize > 1
            ? std::min(4, scaleSize - 1)
            : 0;

    for (std::size_t index = 0;
         index < structure.sections.size();
         ++index)
    {
        const auto& section =
            structure.sections[index];

        HarmonyEvent event;
        event.startBeat = section.startBeat;
        event.endBeat = section.endBeat;
        event.tension = section.tension;

        switch (section.section)
        {
            case PhraseSection::Opening:
                event.scaleDegree = 0;
                break;

            case PhraseSection::Development:
                event.scaleDegree =
                    (static_cast<int>(index) % 2 == 0)
                        ? std::min(3, scaleSize - 1)
                        : std::min(4, scaleSize - 1);
                break;

            case PhraseSection::Preparation:
                event.scaleDegree = preparationDegree;
                event.tension =
                    std::min(
                        100,
                        section.tension + 10);
                break;

            case PhraseSection::Cadence:
                event.scaleDegree =
                    std::clamp(
                        section.targetScaleDegree,
                        0,
                        scaleSize - 1);
                event.tension =
                    std::max(
                        0,
                        section.tension -
                            context.parameters.cadenceStrength / 2);
                break;
        }

        event.rootPitchClass =
            pitchClasses[
                static_cast<std::size_t>(
                    event.scaleDegree)];

        event.quality =
            inferTriadQuality(
                pitchClasses,
                event.scaleDegree);

        plan.events.push_back(event);
    }

    return plan;
}

} // namespace midigengx::music
