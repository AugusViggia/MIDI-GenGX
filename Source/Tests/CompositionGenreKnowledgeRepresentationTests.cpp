#include "Music/CompositionGenreKnowledgeRepresentation.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

using namespace midigengx::domain;
using namespace midigengx::music;

namespace
{

void expect(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

void testKnownGenreProducesValidRepresentation()
{
    const auto representation =
        buildCompositionGenreKnowledgeRepresentation(
            GenrePreset::ProgressiveHouse);

    expect(
        representation.isValid(),
        "known genre should produce a valid representation");

    expect(
        representation.genre == GenrePreset::ProgressiveHouse,
        "representation should preserve the requested genre");
}

void testPresetValuesAreMaterializedDeterministically()
{
    const auto representation =
        buildCompositionGenreKnowledgeRepresentation(
            GenrePreset::ProgressiveHouse);

    const auto& preset =
        getGenrePresetValues(GenrePreset::ProgressiveHouse);

    expect(
        std::abs(
            representation.features[CompositionGenreKnowledgeRepresentation::Density] -
            preset.density / 100.0) < 1.0e-9,
        "density should be represented from the established genre preset");

    expect(
        std::abs(
            representation.features[CompositionGenreKnowledgeRepresentation::Repetition] -
            preset.repetition / 100.0) < 1.0e-9,
        "repetition should be represented from the established genre preset");

    expect(
        std::abs(
            representation.features[CompositionGenreKnowledgeRepresentation::Tension] -
            preset.tension / 100.0) < 1.0e-9,
        "tension should be represented from the established genre preset");
}

void testAllConcreteGenresRemainNormalized()
{
    for (int value = static_cast<int>(GenrePreset::Custom);
         value < static_cast<int>(GenrePreset::Count);
         ++value)
    {
        const auto genre =
            static_cast<GenrePreset>(value);

        const auto representation =
            buildCompositionGenreKnowledgeRepresentation(genre);

        expect(
            representation.isValid(),
            "every concrete genre preset should remain valid");

        for (const auto feature : representation.features)
        {
            expect(
                feature >= 0.0 && feature <= 1.0,
                "genre representation features must remain normalized");
        }
    }
}

void testInvalidSentinelFailsClosed()
{
    const auto representation =
        buildCompositionGenreKnowledgeRepresentation(
            GenrePreset::Count);

    expect(
        !representation.isValid(),
        "genre sentinel must fail closed");
}

void testDerivedProfilesAreDeterministic()
{
    const auto first =
        buildCompositionGenreKnowledgeRepresentation(
            GenrePreset::Techno);
    const auto second =
        buildCompositionGenreKnowledgeRepresentation(
            GenrePreset::Techno);

    for (std::size_t index = 0;
         index < first.features.size();
         ++index)
    {
        expect(
            std::abs(first.features[index] - second.features[index]) < 1.0e-12,
            "same genre must produce identical representation features");
    }
}

} // namespace

int main()
{
    testKnownGenreProducesValidRepresentation();
    testPresetValuesAreMaterializedDeterministically();
    testAllConcreteGenresRemainNormalized();
    testInvalidSentinelFailsClosed();
    testDerivedProfilesAreDeterministic();

    std::cout
        << "MIDI-GenGX Phase 117 genre knowledge representation tests passed.\n";

    return 0;
}
