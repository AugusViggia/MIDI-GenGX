#include "CompositionMidiHarmony.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <vector>

namespace midigengx::music
{
namespace
{

constexpr std::array<double, 12> majorProfile =
{
    6.35, 2.23, 3.48, 2.33, 4.38, 4.09,
    2.52, 5.19, 2.39, 3.66, 2.29, 2.88
};

constexpr std::array<double, 12> minorProfile =
{
    6.33, 2.68, 3.52, 5.38, 2.60, 3.53,
    2.54, 4.75, 3.98, 2.69, 3.34, 3.17
};

std::array<double, 12> buildPitchClassHistogram(
    const CompositionMidiCorpusRecord& record) noexcept
{
    std::array<double, 12> histogram{};

    for (const auto& note :
         record.notes)
    {
        const auto duration =
            static_cast<double>(
                note.endTick -
                note.startTick);

        histogram[
            static_cast<std::size_t>(
                note.midiNote % 12)] +=
            duration *
            static_cast<double>(
                note.velocity + 1);
    }

    return histogram;
}

double circularCorrelation(
    const std::array<double, 12>& histogram,
    const std::array<double, 12>& profile,
    int tonic) noexcept
{
    double histogramMean = 0.0;
    double profileMean = 0.0;

    for (int index = 0; index < 12; ++index)
    {
        histogramMean +=
            histogram[
                static_cast<std::size_t>(
                    index)];

        profileMean +=
            profile[
                static_cast<std::size_t>(
                    index)];
    }

    histogramMean /= 12.0;
    profileMean /= 12.0;

    double numerator = 0.0;
    double histogramEnergy = 0.0;
    double profileEnergy = 0.0;

    for (int index = 0; index < 12; ++index)
    {
        const auto histogramValue =
            histogram[
                static_cast<std::size_t>(
                    (index + tonic) % 12)] -
            histogramMean;

        const auto profileValue =
            profile[
                static_cast<std::size_t>(
                    index)] -
            profileMean;

        numerator +=
            histogramValue *
            profileValue;

        histogramEnergy +=
            histogramValue *
            histogramValue;

        profileEnergy +=
            profileValue *
            profileValue;
    }

    if (histogramEnergy <= 0.0 ||
        profileEnergy <= 0.0)
    {
        return 0.0;
    }

    return numerator /
           std::sqrt(
               histogramEnergy *
               profileEnergy);
}

int normalizePitchClass(
    int value) noexcept
{
    const auto result =
        value % 12;

    return result < 0
        ? result + 12
        : result;
}

double sectionPitchClassScore(
    const std::array<double, 12>& histogram,
    int root,
    ChordQuality quality) noexcept
{
    static constexpr std::array<
        std::array<int, 3>,
        6> intervals =
    {{
        {0, 4, 7},
        {0, 3, 7},
        {0, 3, 6},
        {0, 4, 8},
        {0, 5, 7},
        {0, 0, 0}
    }};

    const auto qualityIndex =
        static_cast<std::size_t>(
            quality);

    if (qualityIndex >=
            intervals.size() - 1)
    {
        return 0.0;
    }

    double chordEnergy = 0.0;
    double nonChordEnergy = 0.0;

    std::array<bool, 12> chordTones{};

    for (const auto interval :
         intervals[qualityIndex])
    {
        chordTones[
            static_cast<std::size_t>(
                normalizePitchClass(
                    root + interval))] = true;
    }

    for (int pitchClass = 0;
         pitchClass < 12;
         ++pitchClass)
    {
        const auto value =
            histogram[
                static_cast<std::size_t>(
                    pitchClass)];

        if (chordTones[
                static_cast<std::size_t>(
                    pitchClass)])
        {
            chordEnergy += value;
        }
        else
        {
            nonChordEnergy += value;
        }
    }

    const auto total =
        chordEnergy +
        nonChordEnergy;

    if (total <= 0.0)
        return 0.0;

    return chordEnergy / total;
}

ChordQuality inferSectionQuality(
    const std::array<double, 12>& histogram,
    int root) noexcept
{
    struct Candidate
    {
        ChordQuality quality =
            ChordQuality::Unknown;

        double score = 0.0;
    };

    const std::array<ChordQuality, 5> qualities =
    {
        ChordQuality::Major,
        ChordQuality::Minor,
        ChordQuality::Diminished,
        ChordQuality::Augmented,
        ChordQuality::Suspended
    };

    Candidate best;

    for (const auto quality :
         qualities)
    {
        const auto score =
            sectionPitchClassScore(
                histogram,
                root,
                quality);

        if (score > best.score)
        {
            best =
            {
                quality,
                score
            };
        }
    }

    if (best.score < 0.45)
        return ChordQuality::Unknown;

    return best.quality;
}

int scaleDegreeForPitchClass(
    int tonic,
    CompositionMidiScale scale,
    int pitchClass) noexcept
{
    static constexpr std::array<int, 7> majorDegrees =
    {
        0, 2, 4, 5, 7, 9, 11
    };

    static constexpr std::array<int, 7> minorDegrees =
    {
        0, 2, 3, 5, 7, 8, 10
    };

    const auto target =
        normalizePitchClass(
            pitchClass - tonic);

    const auto& degrees =
        scale == CompositionMidiScale::Minor
            ? minorDegrees
            : majorDegrees;

    for (std::size_t index = 0;
         index < degrees.size();
         ++index)
    {
        if (degrees[index] == target)
            return static_cast<int>(index);
    }

    return -1;
}

CompositionMidiKeyEstimate estimateKey(
    const std::array<double, 12>& histogram) noexcept
{
    CompositionMidiKeyEstimate best;
    double bestScore =
        -std::numeric_limits<double>::infinity();

    for (int tonic = 0;
         tonic < 12;
         ++tonic)
    {
        const auto major =
            circularCorrelation(
                histogram,
                majorProfile,
                tonic);

        if (major > bestScore)
        {
            bestScore = major;

            best.tonicPitchClass =
                tonic;

            best.scale =
                CompositionMidiScale::Major;

            best.confidence =
                std::clamp(
                    (major + 1.0) / 2.0,
                    0.0,
                    1.0);
        }

        const auto minor =
            circularCorrelation(
                histogram,
                minorProfile,
                tonic);

        if (minor > bestScore)
        {
            bestScore = minor;

            best.tonicPitchClass =
                tonic;

            best.scale =
                CompositionMidiScale::Minor;

            best.confidence =
                std::clamp(
                    (minor + 1.0) / 2.0,
                    0.0,
                    1.0);
        }
    }

    best.valid =
        bestScore >
            0.15;

    if (!best.valid)
    {
        best.scale =
            CompositionMidiScale::Unknown;

        best.confidence =
            0.0;
    }

    return best;
}

std::array<double, 12>
buildSectionHistogram(
    const CompositionMidiCorpusRecord& record,
    const CompositionMidiSection& section) noexcept
{
    std::array<double, 12> histogram{};

    for (const auto& note :
         record.notes)
    {
        if (note.endTick <= section.startTick ||
            note.startTick >= section.endTick)
        {
            continue;
        }

        const auto clippedStart =
            std::max(
                note.startTick,
                section.startTick);

        const auto clippedEnd =
            std::min(
                note.endTick,
                section.endTick);

        if (clippedEnd <= clippedStart)
            continue;

        histogram[
            static_cast<std::size_t>(
                note.midiNote % 12)] +=
            static_cast<double>(
                clippedEnd -
                clippedStart) *
            static_cast<double>(
                note.velocity + 1);
    }

    return histogram;
}

} // namespace

bool CompositionMidiKeyEstimate::isValid()
    const noexcept
{
    return valid &&
           tonicPitchClass >= 0 &&
           tonicPitchClass < 12 &&
           scale !=
               CompositionMidiScale::Unknown &&
           std::isfinite(confidence) &&
           confidence >= 0.0 &&
           confidence <= 1.0;
}

bool CompositionMidiSectionHarmony::isValid()
    const noexcept
{
    return valid &&
           sectionIndex <
               static_cast<std::size_t>(
                   std::numeric_limits<std::uint32_t>::max()) &&
           rootPitchClass >= 0 &&
           rootPitchClass < 12 &&
           scaleDegree >= 0 &&
           scaleDegree <= 6 &&
           std::isfinite(confidence) &&
           confidence >= 0.0 &&
           confidence <= 1.0;
}

bool CompositionMidiHarmonyAnalysis::isValid()
    const noexcept
{
    if (!valid ||
        !key.isValid() ||
        sections.empty())
    {
        return false;
    }

    for (const auto& section :
         sections)
    {
        if (!section.isValid())
            return false;
    }

    return true;
}

CompositionMidiHarmonyAnalysis
analyzeCompositionMidiHarmony(
    const CompositionMidiCorpusRecord& record,
    const CompositionMidiSectionAnalysis& sections)
    noexcept
{
    CompositionMidiHarmonyAnalysis result;

    if (!record.isValid() ||
        !sections.isValid(
            record.ticksPerQuarterNote))
    {
        return result;
    }

    const auto globalHistogram =
        buildPitchClassHistogram(
            record);

    result.key =
        estimateKey(
            globalHistogram);

    if (!result.key.isValid())
        return result;

    result.sections.reserve(
        sections.sections.size());

    for (const auto& section :
         sections.sections)
    {
        const auto histogram =
            buildSectionHistogram(
                record,
                section);

        auto bestRoot =
            0;

        auto bestQuality =
            ChordQuality::Unknown;

        auto bestScore =
            0.0;

        for (int root = 0;
             root < 12;
             ++root)
        {
            const auto quality =
                inferSectionQuality(
                    histogram,
                    root);

            if (quality ==
                ChordQuality::Unknown)
            {
                continue;
            }

            const auto score =
                sectionPitchClassScore(
                    histogram,
                    root,
                    quality);

            if (score > bestScore)
            {
                bestScore =
                    score;

                bestRoot =
                    root;

                bestQuality =
                    quality;
            }
        }

        const auto scaleDegree =
            scaleDegreeForPitchClass(
                result.key.tonicPitchClass,
                result.key.scale,
                bestRoot);

        CompositionMidiSectionHarmony
            harmony;

        harmony.sectionIndex =
            section.sectionIndex;

        harmony.rootPitchClass =
            bestRoot;

        harmony.scaleDegree =
            scaleDegree >= 0
                ? scaleDegree
                : 0;

        harmony.quality =
            bestQuality;

        harmony.confidence =
            bestScore;

        harmony.valid =
            bestQuality !=
                ChordQuality::Unknown &&
            scaleDegree >= 0;

        if (!harmony.valid)
        {
            // Do not fabricate a harmonic label if the section does not
            // contain enough information to support one.
            harmony.quality =
                ChordQuality::Unknown;
        }

        result.sections.push_back(
            harmony);
    }

    result.valid =
        true;

    if (!result.isValid())
        result.valid = false;

    return result;
}

} // namespace midigengx::music
