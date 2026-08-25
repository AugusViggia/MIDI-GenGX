#include "CompositionRuntimeFeatureAdapter.h"

#include <algorithm>
#include <cmath>

namespace midigengx::music
{
namespace
{

double normalizePercent(
    int value) noexcept
{
    return std::clamp(
        static_cast<double>(value) /
            100.0,
        0.0,
        1.0);
}

double normalizeRange(
    int value,
    int minimum,
    int maximum) noexcept
{
    if (maximum <= minimum)
        return 0.0;

    return std::clamp(
        static_cast<double>(
            value - minimum) /
            static_cast<double>(
                maximum - minimum),
        0.0,
        1.0);
}

double encodeSigned(
    double normalized) noexcept
{
    return std::clamp(
        normalized * 2.0 - 1.0,
        -1.0,
        1.0);
}

} // namespace

bool CompositionRuntimeFeatures::isValid() const noexcept
{
    if (!valid ||
        globalFeatures.size() !=
            CompositionDatasetSchema::globalFeatureCount ||
        sectionFeatures.size() !=
            CompositionDatasetSchema::sectionFeatureCount)
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

    for (const auto value :
         sectionFeatures)
    {
        if (!std::isfinite(value) ||
            value < -1.0 ||
            value > 1.0)
        {
            return false;
        }
    }

    return true;
}

CompositionRuntimeFeatures
CompositionRuntimeFeatureAdapter::build(
    const midigengx::domain::MusicalContext& inputContext)
    const noexcept
{
    CompositionRuntimeFeatures result;

    auto context =
        inputContext;

    context.normalize();

    result.globalFeatures.assign(
        CompositionDatasetSchema::globalFeatureCount,
        0.0);

    result.sectionFeatures.assign(
        CompositionDatasetSchema::sectionFeatureCount,
        0.0);

    const auto& parameters =
        context.parameters;

    // Runtime-safe representation:
    // structure-derived global values that are not available at generation
    // time remain neutral rather than being fabricated from unrelated data.
    result.globalFeatures[
        CompositionDatasetSchema::globalFeatureIndex(
            GlobalFeature::TotalLengthNormalized)] =
        normalizeRange(
            parameters.lengthBars,
            1,
            64);

    result.globalFeatures[
        CompositionDatasetSchema::globalFeatureIndex(
            GlobalFeature::SectionCountNormalized)] =
        normalizeRange(
            parameters.lengthBars,
            1,
            64);

    result.globalFeatures[
        CompositionDatasetSchema::globalFeatureIndex(
            GlobalFeature::AverageSectionTensionNormalized)] =
        normalizePercent(
            parameters.tension);

    result.globalFeatures[
        CompositionDatasetSchema::globalFeatureIndex(
            GlobalFeature::MinimumSectionTensionNormalized)] =
        normalizePercent(
            parameters.tension);

    result.globalFeatures[
        CompositionDatasetSchema::globalFeatureIndex(
            GlobalFeature::MaximumSectionTensionNormalized)] =
        normalizePercent(
            parameters.tension);

    result.globalFeatures[
        CompositionDatasetSchema::globalFeatureIndex(
            GlobalFeature::PeakTensionNormalized)] =
        normalizePercent(
            parameters.tension);

    result.sectionFeatures[
        CompositionDatasetSchema::sectionFeatureIndex(
            SectionFeature::RoleEncoded)] =
        encodeSigned(
            normalizeRange(
                static_cast<int>(
                    context.role),
                0,
                8));

    result.sectionFeatures[
        CompositionDatasetSchema::sectionFeatureIndex(
            SectionFeature::TensionNormalized)] =
        normalizePercent(
            parameters.tension);

    // Runtime has no prior generated section here; deltas are therefore
    // neutral until the coordinator supplies actual previous-section state.
    result.sectionFeatures[
        CompositionDatasetSchema::sectionFeatureIndex(
            SectionFeature::TensionDeltaNormalized)] =
        0.0;

    result.sectionFeatures[
        CompositionDatasetSchema::sectionFeatureIndex(
            SectionFeature::HarmonyScaleDegreeNormalized)] =
        0.0;

    result.sectionFeatures[
        CompositionDatasetSchema::sectionFeatureIndex(
            SectionFeature::HarmonyQualityEncoded)] =
        encodeSigned(
            normalizePercent(
                parameters.complexity));

    result.sectionFeatures[
        CompositionDatasetSchema::sectionFeatureIndex(
            SectionFeature::HarmonicDegreeDeltaNormalized)] =
        0.0;

    result.valid = true;

    return result;
}

} // namespace midigengx::music
