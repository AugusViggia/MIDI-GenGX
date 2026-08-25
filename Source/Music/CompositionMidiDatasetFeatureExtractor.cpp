#include "CompositionMidiDatasetFeatureExtractor.h"

#include <algorithm>
#include <cmath>

namespace midigengx::music
{
namespace
{

double normalizeRange(
    double value,
    double minimum,
    double maximum) noexcept
{
    if (maximum <= minimum)
        return 0.0;

    return std::clamp(
        (value - minimum) /
            (maximum - minimum),
        0.0,
        1.0);
}

double normalizeSigned(
    double value,
    double magnitude) noexcept
{
    if (magnitude <= 0.0)
        return 0.0;

    return std::clamp(
        value / magnitude,
        -1.0,
        1.0);
}

double encodeRole(
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

double encodeUnknownHarmonyQuality() noexcept
{
    return 1.0;
}

double harmonicQualityEncoding(
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

CompositionDatasetSample
CompositionMidiDatasetFeatureExtractor::buildSample(
    const CompositionMidiCorpusRecord& record,
    const CompositionMidiCorpusAnalysis& analysis,
    const CompositionMidiSectionAnalysis& sections,
    const CompositionMidiHarmonyAnalysis& harmony,
    const CompositionMidiMotifAnalysis& motifs,
    const std::string& sampleId) const noexcept
{
    CompositionDatasetSample sample;

    if (!record.isValid() ||
        !analysis.isValid() ||
        !sections.isValid(
            record.ticksPerQuarterNote) ||
        !harmony.isValid() ||
        !motifs.isValid() ||
        sampleId.empty())
    {
        return sample;
    }

    if (sections.sections.empty())
        return sample;

    const auto totalLengthBeats =
        static_cast<double>(
            record.lengthTicks) /
        static_cast<double>(
            record.ticksPerQuarterNote);

    double averageTension = 0.0;
    int minimumTension = 100;
    int maximumTension = 0;

    int risingTransitions = 0;
    int fallingTransitions = 0;
    int flatTransitions = 0;
    int peakTension = 0;

    for (std::size_t index = 0;
         index < sections.sections.size();
         ++index)
    {
        const auto tension =
            sections.sections[index].tension;

        averageTension +=
            static_cast<double>(
                tension);

        minimumTension =
            std::min(
                minimumTension,
                tension);

        maximumTension =
            std::max(
                maximumTension,
                tension);

        peakTension =
            std::max(
                peakTension,
                tension);

        if (index == 0)
            continue;

        const auto previous =
            sections.sections[index - 1].tension;

        if (tension > previous)
            ++risingTransitions;
        else if (tension < previous)
            ++fallingTransitions;
        else
            ++flatTransitions;
    }

    averageTension /=
        static_cast<double>(
            sections.sections.size());

    sample.sampleId =
        sampleId;

    sample.globalFeatures =
    {
        std::clamp(
            totalLengthBeats /
                1024.0,
            0.0,
            1.0),

        normalizeRange(
            static_cast<double>(
                sections.sectionCount()),
            1.0,
            64.0),

        std::clamp(
            static_cast<double>(
                harmony.sections.size()) /
                64.0,
            0.0,
            1.0),

        std::clamp(
            static_cast<double>(
                motifs.totalFamilyCount()) /
                64.0,
            0.0,
            1.0),

        std::clamp(
            static_cast<double>(
                motifs.recurringFamilyCount()) /
                64.0,
            0.0,
            1.0),

        std::clamp(
            motifs.averageOccurrenceCount() /
                16.0,
            0.0,
            1.0),

        std::clamp(
            averageTension /
                100.0,
            0.0,
            1.0),

        std::clamp(
            static_cast<double>(
                minimumTension) /
                100.0,
            0.0,
            1.0),

        std::clamp(
            static_cast<double>(
                maximumTension) /
                100.0,
            0.0,
            1.0),

        normalizeRange(
            static_cast<double>(
                risingTransitions),
            0.0,
            64.0),

        normalizeRange(
            static_cast<double>(
                fallingTransitions),
            0.0,
            64.0),

        normalizeRange(
            static_cast<double>(
                flatTransitions),
            0.0,
            64.0),

        std::clamp(
            static_cast<double>(
                peakTension) /
                100.0,
            0.0,
            1.0)
    };

    sample.sectionFeatures.reserve(
        sections.sections.size());

    for (std::size_t index = 0;
         index < sections.sections.size();
         ++index)
    {
        const auto& section =
            sections.sections[index];

        const auto tensionDelta =
            index == 0
                ? 0.0
                : static_cast<double>(
                      section.tension -
                      sections.sections[
                          index - 1].tension);

        const auto* harmonicSection =
            index < harmony.sections.size()
                ? &harmony.sections[index]
                : nullptr;

        const auto harmonyScaleDegree =
            harmonicSection != nullptr &&
            harmonicSection->valid
                ? static_cast<double>(
                      harmonicSection->scaleDegree) /
                  6.0
                : 0.0;

        const auto harmonyQuality =
            harmonicSection != nullptr &&
            harmonicSection->valid
                ? harmonicQualityEncoding(
                      harmonicSection->quality)
                : encodeUnknownHarmonyQuality();

        const auto harmonicDegreeDelta =
            index == 0 ||
            harmonicSection == nullptr ||
            !harmonicSection->valid ||
            index - 1 >= harmony.sections.size() ||
            !harmony.sections[index - 1].valid
                ? 0.0
                : normalizeSigned(
                      static_cast<double>(
                          harmonicSection->scaleDegree -
                          harmony.sections[index - 1].scaleDegree),
                      6.0);

        sample.sectionFeatures.push_back(
        {
            encodeRole(
                section.role),

            std::clamp(
                static_cast<double>(
                    section.tension) /
                100.0,
                0.0,
                1.0),

            normalizeSigned(
                tensionDelta,
                100.0),

            harmonyScaleDegree,

            harmonyQuality,

            harmonicDegreeDelta
        });
    }

    sample.analysisValid =
        true;

    return sample;
}

} // namespace midigengx::music
