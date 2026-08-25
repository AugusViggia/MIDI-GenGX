#pragma once

#include "CompositionDatasetSample.h"

#include <cstddef>

namespace midigengx::music
{

enum class GlobalFeature
{
    TotalLengthNormalized = 0,
    SectionCountNormalized,
    HarmonyEventCountNormalized,
    MotifFamilyCountNormalized,
    RecurringMotifFamilyCountNormalized,
    AverageMotifOccurrencesNormalized,
    AverageSectionTensionNormalized,
    MinimumSectionTensionNormalized,
    MaximumSectionTensionNormalized,
    RisingTransitionsNormalized,
    FallingTransitionsNormalized,
    FlatTransitionsNormalized,
    PeakTensionNormalized
};

enum class SectionFeature
{
    RoleEncoded = 0,
    TensionNormalized,
    TensionDeltaNormalized,
    HarmonyScaleDegreeNormalized,
    HarmonyQualityEncoded,
    HarmonicDegreeDeltaNormalized
};

struct CompositionDatasetSchema
{
    static constexpr int version = 1;
    static constexpr std::size_t globalFeatureCount = 13;
    static constexpr std::size_t sectionFeatureCount = 6;

    static constexpr const char* name =
        "MIDI-GenGX Composition Dataset";

    static const char* globalFeatureName(
        GlobalFeature feature) noexcept;

    static const char* sectionFeatureName(
        SectionFeature feature) noexcept;

    static std::size_t globalFeatureIndex(
        GlobalFeature feature) noexcept;

    static std::size_t sectionFeatureIndex(
        SectionFeature feature) noexcept;

    static bool validateSample(
        const CompositionDatasetSample& sample) noexcept;
};

} // namespace midigengx::music
