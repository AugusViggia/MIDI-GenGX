#include "Music/CompositionSoundEngineeringKnowledgeRepresentation.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>

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

void testDefaultIntentProducesValidRepresentation()
{
    const SoundEngineeringIntent intent{};

    const auto representation =
        buildCompositionSoundEngineeringKnowledgeRepresentation(intent);

    expect(
        representation.isValid(),
        "default sound engineering intent should produce a valid representation");

    expect(
        representation.features[
            CompositionSoundEngineeringKnowledgeRepresentation::PrioritizeClarity] == 1.0,
        "default clarity preference should be enabled");

    expect(
        representation.features[
            CompositionSoundEngineeringKnowledgeRepresentation::ProtectLowEnd] == 1.0,
        "default low-end protection should be enabled");
}

void testNumericIntentValuesAreMaterializedDeterministically()
{
    SoundEngineeringIntent intent{};
    intent.lowEndOrganization = 10;
    intent.registerSeparation = 20;
    intent.rhythmicSpace = 30;
    intent.densityBudget = 40;
    intent.energyControl = 50;
    intent.grooveFocus = 60;
    intent.prioritizeClarity = false;
    intent.protectLowEnd = true;

    const auto representation =
        buildCompositionSoundEngineeringKnowledgeRepresentation(intent);

    expect(
        std::abs(
            representation.features[
                CompositionSoundEngineeringKnowledgeRepresentation::LowEndOrganization] -
            0.10) < 1.0e-12,
        "low-end organization should be normalized from the established intent");

    expect(
        std::abs(
            representation.features[
                CompositionSoundEngineeringKnowledgeRepresentation::DensityBudget] -
            0.40) < 1.0e-12,
        "density budget should be normalized from the established intent");

    expect(
        std::abs(
            representation.features[
                CompositionSoundEngineeringKnowledgeRepresentation::GrooveFocus] -
            0.60) < 1.0e-12,
        "groove focus should be normalized from the established intent");

    expect(
        representation.features[
            CompositionSoundEngineeringKnowledgeRepresentation::PrioritizeClarity] == 0.0,
        "clarity preference should be encoded as zero when disabled");

    expect(
        representation.features[
            CompositionSoundEngineeringKnowledgeRepresentation::ProtectLowEnd] == 1.0,
        "low-end protection should be encoded as one when enabled");
}

void testOutOfRangeIntentValuesClampThroughRepresentationBoundary()
{
    SoundEngineeringIntent intent{};
    intent.lowEndOrganization = -20;
    intent.registerSeparation = 120;
    intent.rhythmicSpace = -1;
    intent.densityBudget = 101;
    intent.energyControl = 300;
    intent.grooveFocus = -300;

    const auto representation =
        buildCompositionSoundEngineeringKnowledgeRepresentation(intent);

    expect(
        representation.isValid(),
        "representation should remain valid after boundary clamping");

    expect(
        representation.features[
            CompositionSoundEngineeringKnowledgeRepresentation::LowEndOrganization] == 0.0,
        "negative low-end value should clamp to zero");

    expect(
        representation.features[
            CompositionSoundEngineeringKnowledgeRepresentation::RegisterSeparation] == 1.0,
        "high register separation value should clamp to one");

    expect(
        representation.features[
            CompositionSoundEngineeringKnowledgeRepresentation::EnergyControl] == 1.0,
        "high energy control value should clamp to one");

    expect(
        representation.features[
            CompositionSoundEngineeringKnowledgeRepresentation::GrooveFocus] == 0.0,
        "negative groove focus value should clamp to zero");
}

void testInvalidFeatureValuesFailClosed()
{
    auto representation =
        buildCompositionSoundEngineeringKnowledgeRepresentation(
            SoundEngineeringIntent{});

    representation.features[0] = -0.01;
    expect(
        !representation.isValid(),
        "negative representation features must fail validation");

    representation =
        buildCompositionSoundEngineeringKnowledgeRepresentation(
            SoundEngineeringIntent{});
    representation.features[0] = std::numeric_limits<double>::quiet_NaN();
    expect(
        !representation.isValid(),
        "non-finite representation features must fail validation");

    representation =
        buildCompositionSoundEngineeringKnowledgeRepresentation(
            SoundEngineeringIntent{});
    representation.valid = false;
    expect(
        !representation.isValid(),
        "invalid representation flag must fail validation");
}

void testSameIntentProducesIdenticalRepresentation()
{
    SoundEngineeringIntent intent{};
    intent.lowEndOrganization = 83;
    intent.registerSeparation = 61;
    intent.rhythmicSpace = 42;
    intent.densityBudget = 37;
    intent.energyControl = 74;
    intent.grooveFocus = 58;
    intent.prioritizeClarity = false;
    intent.protectLowEnd = true;

    const auto first =
        buildCompositionSoundEngineeringKnowledgeRepresentation(intent);
    const auto second =
        buildCompositionSoundEngineeringKnowledgeRepresentation(intent);

    expect(
        first.isValid() && second.isValid(),
        "identical inputs should produce valid representations");

    for (std::size_t index = 0;
         index < first.features.size();
         ++index)
    {
        expect(
            std::abs(first.features[index] - second.features[index]) < 1.0e-12,
            "same sound engineering intent must produce identical features");
    }
}

} // namespace

int main()
{
    testDefaultIntentProducesValidRepresentation();
    testNumericIntentValuesAreMaterializedDeterministically();
    testOutOfRangeIntentValuesClampThroughRepresentationBoundary();
    testInvalidFeatureValuesFailClosed();
    testSameIntentProducesIdenticalRepresentation();

    std::cout
        << "MIDI-GenGX Phase 118 sound engineering knowledge representation tests passed.\n";

    return 0;
}
