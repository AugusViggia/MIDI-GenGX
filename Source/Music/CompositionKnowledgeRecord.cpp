#include "CompositionKnowledgeRecord.h"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace midigengx::music
{

bool CompositionKnowledgeRecord::isValid() const noexcept
{
    if (totalLengthBeats <= 0.0 ||
        !std::isfinite(totalLengthBeats) ||
        sectionCount == 0 ||
        sectionCount != sectionRoles.size() ||
        sectionCount != sectionTensions.size() ||
        harmonyEventCount == 0 ||
        recurringMotifFamilyCount >
            totalMotifFamilyCount ||
        !std::isfinite(averageMotifOccurrences) ||
        averageMotifOccurrences < 0.0 ||
        !std::isfinite(averageSectionTension) ||
        averageSectionTension < 0.0 ||
        averageSectionTension > 100.0 ||
        minimumSectionTension < 0 ||
        maximumSectionTension > 100 ||
        minimumSectionTension >
            maximumSectionTension)
    {
        return false;
    }

    for (const auto tension :
         sectionTensions)
    {
        if (tension < 0 ||
            tension > 100)
        {
            return false;
        }
    }

    const auto knownHarmony =
        majorHarmonyEvents +
        minorHarmonyEvents +
        diminishedHarmonyEvents +
        augmentedHarmonyEvents +
        suspendedHarmonyEvents +
        unknownHarmonyEvents;

    if (knownHarmony !=
        harmonyEventCount)
    {
        return false;
    }

    return true;
}

CompositionKnowledgeRecord
buildCompositionKnowledgeRecord(
    const PhraseStructurePlan& structure,
    const HarmonyPlan& harmony,
    const MotifKnowledgeCatalog& motifs) noexcept
{
    CompositionKnowledgeRecord record;

    if (!structure.isValid() ||
        !harmony.isValid() ||
        !motifs.isValid() ||
        std::abs(
            structure.totalLengthBeats -
            harmony.totalLengthBeats) >
            1.0e-9)
    {
        return record;
    }

    record.totalLengthBeats =
        structure.totalLengthBeats;

    record.sectionCount =
        structure.sections.size();

    record.harmonyEventCount =
        harmony.events.size();

    record.sectionRoles.reserve(
        structure.sections.size());

    record.sectionTensions.reserve(
        structure.sections.size());

    int tensionTotal = 0;

    record.minimumSectionTension =
        structure.sections.front().tension;

    record.maximumSectionTension =
        structure.sections.front().tension;

    for (const auto& section :
         structure.sections)
    {
        record.sectionRoles.push_back(
            section.section);

        record.sectionTensions.push_back(
            section.tension);

        tensionTotal +=
            section.tension;

        record.minimumSectionTension =
            std::min(
                record.minimumSectionTension,
                section.tension);

        record.maximumSectionTension =
            std::max(
                record.maximumSectionTension,
                section.tension);
    }

    record.averageSectionTension =
        static_cast<double>(
            tensionTotal) /
        static_cast<double>(
            record.sectionCount);

    record.recurringMotifFamilyCount =
        motifs.recurringCount();

    record.totalMotifFamilyCount =
        motifs.size();

    record.averageMotifOccurrences =
        motifs.averageOccurrenceCount();

    for (const auto& event :
         harmony.events)
    {
        switch (event.quality)
        {
            case ChordQuality::Major:
                ++record.majorHarmonyEvents;
                break;

            case ChordQuality::Minor:
                ++record.minorHarmonyEvents;
                break;

            case ChordQuality::Diminished:
                ++record.diminishedHarmonyEvents;
                break;

            case ChordQuality::Augmented:
                ++record.augmentedHarmonyEvents;
                break;

            case ChordQuality::Suspended:
                ++record.suspendedHarmonyEvents;
                break;

            case ChordQuality::Unknown:
                ++record.unknownHarmonyEvents;
                break;
        }
    }

    return record;
}

} // namespace midigengx::music
