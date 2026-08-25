#include "CompositionDatasetSample.h"

#include <algorithm>
#include <cmath>

namespace midigengx::music
{
namespace
{

double normalizeCount(
    std::size_t value,
    std::size_t upperBound) noexcept
{
    if (upperBound == 0)
        return 0.0;

    return std::clamp(
        static_cast<double>(value) /
            static_cast<double>(upperBound),
        0.0,
        1.0);
}

double normalizeTension(
    int value) noexcept
{
    return std::clamp(
        static_cast<double>(value) /
            100.0,
        0.0,
        1.0);
}

double normalizeSignedDelta(
    int value) noexcept
{
    return std::clamp(
        static_cast<double>(value) /
            100.0,
        -1.0,
        1.0);
}

double encodeSectionRole(
    PhraseSection role) noexcept
{
    switch (role)
    {
        case PhraseSection::Opening:
            return 0.0;

        case PhraseSection::Development:
            return 1.0 / 3.0;

        case PhraseSection::Preparation:
            return 2.0 / 3.0;

        case PhraseSection::Cadence:
            return 1.0;
    }

    return 0.0;
}

double encodeChordQuality(
    ChordQuality quality) noexcept
{
    switch (quality)
    {
        case ChordQuality::Major:
            return 0.0;

        case ChordQuality::Minor:
            return 0.2;

        case ChordQuality::Diminished:
            return 0.4;

        case ChordQuality::Augmented:
            return 0.6;

        case ChordQuality::Suspended:
            return 0.8;

        case ChordQuality::Unknown:
            return 1.0;
    }

    return 1.0;
}

} // namespace

bool CompositionDatasetSample::isValid() const noexcept
{
    if (!analysisValid ||
        sampleId.empty() ||
        globalFeatures.empty())
    {
        return false;
    }

    for (const auto value :
         globalFeatures)
    {
        if (!std::isfinite(value) ||
            value < -1.0 ||
            value > 1.0)
        {
            return false;
        }
    }

    for (const auto& section :
         sectionFeatures)
    {
        if (section.empty())
            return false;

        for (const auto value :
             section)
        {
            if (!std::isfinite(value) ||
                value < -1.0 ||
                value > 1.0)
            {
                return false;
            }
        }
    }

    return true;
}

std::size_t
CompositionDatasetSample::sectionCount() const noexcept
{
    return sectionFeatures.size();
}

CompositionDatasetSample
buildCompositionDatasetSample(
    const CompositionKnowledgeSnapshot& snapshot,
    const std::string& sampleId) noexcept
{
    CompositionDatasetSample sample;

    if (!snapshot.isValid() ||
        sampleId.empty())
    {
        return sample;
    }

    sample.analysisValid = true;
    sample.sampleId = sampleId;

    const auto& composition =
        snapshot.composition;

    const auto& transitions =
        snapshot.transitions;

    // Fixed order. This order is part of schemaVersion 1.
    sample.globalFeatures =
    {
        std::clamp(
            composition.totalLengthBeats /
                1024.0,
            0.0,
            1.0),

        normalizeCount(
            composition.sectionCount,
            64),

        normalizeCount(
            composition.harmonyEventCount,
            128),

        normalizeCount(
            composition.totalMotifFamilyCount,
            64),

        normalizeCount(
            composition.recurringMotifFamilyCount,
            64),

        std::clamp(
            composition.averageMotifOccurrences /
                16.0,
            0.0,
            1.0),

        normalizeTension(
            composition.averageSectionTension),

        normalizeTension(
            composition.minimumSectionTension),

        normalizeTension(
            composition.maximumSectionTension),

        normalizeCount(
            transitions.risingTransitions,
            64),

        normalizeCount(
            transitions.fallingTransitions,
            64),

        normalizeCount(
            transitions.flatTransitions,
            64),

        normalizeTension(
            transitions.peakTension)
    };

    sample.sectionFeatures.reserve(
        snapshot.sections.size());

    for (const auto& section :
         snapshot.sections)
    {
        sample.sectionFeatures.push_back(
        {
            encodeSectionRole(
                section.role),

            normalizeTension(
                section.tension),

            normalizeSignedDelta(
                section.tensionDeltaFromPrevious),

            std::clamp(
                static_cast<double>(
                    section.harmonyScaleDegree) /
                    12.0,
                0.0,
                1.0),

            encodeChordQuality(
                section.harmonyQuality),

            normalizeSignedDelta(
                section.harmonicDegreeDeltaFromPrevious)
        });
    }

    return sample;
}

} // namespace midigengx::music
