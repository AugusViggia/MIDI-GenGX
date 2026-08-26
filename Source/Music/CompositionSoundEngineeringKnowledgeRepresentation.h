#pragma once

#include "../Domain/CompositionIntent.h"

#include <array>
#include <cstddef>

namespace midigengx::music
{

struct CompositionSoundEngineeringKnowledgeRepresentation
{
    static constexpr int version = 1;
    static constexpr std::size_t featureCount = 8;

    enum Feature : std::size_t
    {
        LowEndOrganization = 0,
        RegisterSeparation,
        RhythmicSpace,
        DensityBudget,
        EnergyControl,
        GrooveFocus,
        PrioritizeClarity,
        ProtectLowEnd
    };

    std::array<double, featureCount> features{};
    bool valid = false;

    bool isValid() const noexcept;
};

CompositionSoundEngineeringKnowledgeRepresentation
buildCompositionSoundEngineeringKnowledgeRepresentation(
    const midigengx::domain::SoundEngineeringIntent& intent) noexcept;

} // namespace midigengx::music
