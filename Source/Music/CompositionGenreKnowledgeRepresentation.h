#pragma once

#include "../Domain/GenrePresets.h"

#include <array>
#include <cstddef>

namespace midigengx::music
{

struct CompositionGenreKnowledgeRepresentation
{
    static constexpr int version = 1;
    static constexpr std::size_t featureCount = 15;

    enum Feature : std::size_t
    {
        Density = 0,
        Variation,
        Complexity,
        Syncopation,
        Tension,
        Repetition,
        Humanization,
        NoteLengthVariation,
        CadenceStrength,
        NoteLengthEncoding,
        PhraseContourEncoding,
        CadenceStyleEncoding,
        EnergyProfile,
        StructuralProfile,
        HarmonicProfile
    };

    midigengx::domain::GenrePreset genre =
        midigengx::domain::GenrePreset::Custom;

    std::array<double, featureCount> features{};
    bool valid = false;

    bool isValid() const noexcept;
};

CompositionGenreKnowledgeRepresentation
buildCompositionGenreKnowledgeRepresentation(
    midigengx::domain::GenrePreset genre) noexcept;

} // namespace midigengx::music
