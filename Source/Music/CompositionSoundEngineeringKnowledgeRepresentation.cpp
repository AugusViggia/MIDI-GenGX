#include "CompositionSoundEngineeringKnowledgeRepresentation.h"

#include <algorithm>
#include <cmath>

namespace midigengx::music
{
namespace
{

bool isUnit(double value) noexcept
{
    return std::isfinite(value) &&
           value >= 0.0 &&
           value <= 1.0;
}

double percent(int value) noexcept
{
    return std::clamp(value, 0, 100) / 100.0;
}

double booleanValue(bool value) noexcept
{
    return value ? 1.0 : 0.0;
}

} // namespace

bool CompositionSoundEngineeringKnowledgeRepresentation::isValid()
    const noexcept
{
    if (!valid)
        return false;

    return std::all_of(
        features.begin(),
        features.end(),
        [](double value)
        {
            return isUnit(value);
        });
}

CompositionSoundEngineeringKnowledgeRepresentation
buildCompositionSoundEngineeringKnowledgeRepresentation(
    const midigengx::domain::SoundEngineeringIntent& intent) noexcept
{
    CompositionSoundEngineeringKnowledgeRepresentation result;

    result.features[
        CompositionSoundEngineeringKnowledgeRepresentation::LowEndOrganization] =
        percent(intent.lowEndOrganization);

    result.features[
        CompositionSoundEngineeringKnowledgeRepresentation::RegisterSeparation] =
        percent(intent.registerSeparation);

    result.features[
        CompositionSoundEngineeringKnowledgeRepresentation::RhythmicSpace] =
        percent(intent.rhythmicSpace);

    result.features[
        CompositionSoundEngineeringKnowledgeRepresentation::DensityBudget] =
        percent(intent.densityBudget);

    result.features[
        CompositionSoundEngineeringKnowledgeRepresentation::EnergyControl] =
        percent(intent.energyControl);

    result.features[
        CompositionSoundEngineeringKnowledgeRepresentation::GrooveFocus] =
        percent(intent.grooveFocus);

    result.features[
        CompositionSoundEngineeringKnowledgeRepresentation::PrioritizeClarity] =
        booleanValue(intent.prioritizeClarity);

    result.features[
        CompositionSoundEngineeringKnowledgeRepresentation::ProtectLowEnd] =
        booleanValue(intent.protectLowEnd);

    result.valid = true;

    if (!result.isValid())
        result.valid = false;

    return result;
}

} // namespace midigengx::music
