#include "MotifRecurrenceProfile.h"

#include <algorithm>
#include <string>
#include <unordered_map>

namespace midigengx::music
{

std::size_t MotifRecurrenceFamily::transformationCount() const noexcept
{
    return transpositionCount +
           retrogradeCount +
           inversionCount +
           rhythmicVariationCount +
           intervalVariationCount +
           compoundVariationCount;
}

bool MotifRecurrenceFamily::isRecurring() const noexcept
{
    return occurrenceIndices.size() > 1;
}

bool MotifRecurrenceProfile::isValid() const noexcept
{
    if (!analysisValid)
        return false;

    for (std::size_t i = 0;
         i < families.size();
         ++i)
    {
        const auto& family = families[i];

        if (!family.fingerprint.isValid() ||
            family.familyIndex != i ||
            family.occurrenceIndices.empty() ||
            family.occurrenceIndices.size() !=
                family.phraseIndices.size() ||
            family.firstPhraseIndex >
                family.lastPhraseIndex)
        {
            return false;
        }

    }

    return true;
}

const MotifRecurrenceFamily*
MotifRecurrenceProfile::findFamily(
    const MotifFingerprint& fingerprint) const noexcept
{
    if (!fingerprint.isValid())
        return nullptr;

    for (const auto& family :
         families)
    {
        if (isMotivicIdentityEquivalent(
                family.fingerprint,
                fingerprint))
        {
            return &family;
        }
    }

    return nullptr;
}

MotifRecurrenceProfile analyzeMotifRecurrence(
    const MotifOccurrenceGraph& graph) noexcept
{
    MotifRecurrenceProfile profile;

    if (!graph.isValid())
        return profile;

    profile.analysisValid = true;

    std::unordered_map<
        std::string,
        std::size_t> familyByKey;

    for (const auto& occurrence :
         graph.occurrences)
    {
        const auto key =
            occurrence.fingerprint.canonicalKey();

        const auto [it, inserted] =
            familyByKey.emplace(
                key,
                profile.families.size());

        if (inserted)
        {
            MotifRecurrenceFamily family;
            family.familyIndex = profile.families.size();
            family.fingerprint =
                occurrence.fingerprint;
            family.firstPhraseIndex =
                occurrence.sourcePhraseIndex;
            family.lastPhraseIndex =
                occurrence.sourcePhraseIndex;

            profile.families.push_back(
                family);
        }

        auto& family =
            profile.families[it->second];

        family.occurrenceIndices.push_back(
            occurrence.occurrenceIndex);

        family.phraseIndices.push_back(
            occurrence.sourcePhraseIndex);

        family.firstPhraseIndex =
            std::min(
                family.firstPhraseIndex,
                occurrence.sourcePhraseIndex);

        family.lastPhraseIndex =
            std::max(
                family.lastPhraseIndex,
                occurrence.sourcePhraseIndex);
    }

    for (const auto& edge :
         graph.edges)
    {
        if (edge.sourceOccurrence >=
                graph.occurrences.size() ||
            edge.targetOccurrence >=
                graph.occurrences.size())
        {
            continue;
        }

        const auto& sourceOccurrence =
            graph.occurrences[
                edge.sourceOccurrence];

        const auto key =
            sourceOccurrence.fingerprint.canonicalKey();

        const auto it =
            familyByKey.find(key);

        if (it == familyByKey.end())
            continue;

        auto& family =
            profile.families[it->second];

        switch (edge.relationship.kind)
        {
            case MotifRelationshipKind::Transposition:
                ++family.transpositionCount;
                break;

            case MotifRelationshipKind::Retrograde:
                ++family.retrogradeCount;
                break;

            case MotifRelationshipKind::Inversion:
                ++family.inversionCount;
                break;

            case MotifRelationshipKind::RhythmicVariation:
                ++family.rhythmicVariationCount;
                break;

            case MotifRelationshipKind::IntervalVariation:
                ++family.intervalVariationCount;
                break;

            case MotifRelationshipKind::CompoundVariation:
                ++family.compoundVariationCount;
                break;

            case MotifRelationshipKind::Identity:
            case MotifRelationshipKind::Invalid:
                break;
        }
    }

    return profile;
}

} // namespace midigengx::music
