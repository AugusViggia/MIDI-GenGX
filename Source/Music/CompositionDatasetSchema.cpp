#include "CompositionDatasetSchema.h"

namespace midigengx::music
{

const char*
CompositionDatasetSchema::globalFeatureName(
    GlobalFeature feature) noexcept
{
    switch (feature)
    {
        case GlobalFeature::TotalLengthNormalized:
            return "total_length";

        case GlobalFeature::SectionCountNormalized:
            return "section_count";

        case GlobalFeature::HarmonyEventCountNormalized:
            return "harmony_event_count";

        case GlobalFeature::MotifFamilyCountNormalized:
            return "motif_family_count";

        case GlobalFeature::RecurringMotifFamilyCountNormalized:
            return "recurring_motif_family_count";

        case GlobalFeature::AverageMotifOccurrencesNormalized:
            return "average_motif_occurrences";

        case GlobalFeature::AverageSectionTensionNormalized:
            return "average_section_tension";

        case GlobalFeature::MinimumSectionTensionNormalized:
            return "minimum_section_tension";

        case GlobalFeature::MaximumSectionTensionNormalized:
            return "maximum_section_tension";

        case GlobalFeature::RisingTransitionsNormalized:
            return "rising_transitions";

        case GlobalFeature::FallingTransitionsNormalized:
            return "falling_transitions";

        case GlobalFeature::FlatTransitionsNormalized:
            return "flat_transitions";

        case GlobalFeature::PeakTensionNormalized:
            return "peak_tension";
    }

    return "";
}

const char*
CompositionDatasetSchema::sectionFeatureName(
    SectionFeature feature) noexcept
{
    switch (feature)
    {
        case SectionFeature::RoleEncoded:
            return "role";

        case SectionFeature::TensionNormalized:
            return "tension";

        case SectionFeature::TensionDeltaNormalized:
            return "tension_delta";

        case SectionFeature::HarmonyScaleDegreeNormalized:
            return "harmony_scale_degree";

        case SectionFeature::HarmonyQualityEncoded:
            return "harmony_quality";

        case SectionFeature::HarmonicDegreeDeltaNormalized:
            return "harmonic_degree_delta";
    }

    return "";
}

std::size_t CompositionDatasetSchema::globalFeatureIndex(
    GlobalFeature feature) noexcept
{
    return static_cast<std::size_t>(
        feature);
}

std::size_t CompositionDatasetSchema::sectionFeatureIndex(
    SectionFeature feature) noexcept
{
    return static_cast<std::size_t>(
        feature);
}

bool CompositionDatasetSchema::validateSample(
    const CompositionDatasetSample& sample) noexcept
{
    if (!sample.isValid() ||
        sample.schemaVersion != version ||
        sample.globalFeatures.size() !=
            globalFeatureCount)
    {
        return false;
    }

    for (const auto& section :
         sample.sectionFeatures)
    {
        if (section.size() !=
            sectionFeatureCount)
        {
            return false;
        }
    }

    return true;
}

} // namespace midigengx::music
