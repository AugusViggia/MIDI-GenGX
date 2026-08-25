#include "Music/CompositionAIEngineBridge.h"

#include <cstdlib>
#include <iostream>

using namespace midigengx::music;

namespace
{
void expect(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr
            << "FAILED: "
            << message
            << '\n';
        std::exit(1);
    }
}

CompositionAIConstraintProfile buildProfile()
{
    CompositionAIConstraintProfile profile;

    profile.roleTarget = 0.20;
    profile.tensionTarget = 0.45;
    profile.tensionDeltaTarget = 0.10;
    profile.harmonyDegreeTarget = -0.15;
    profile.harmonyQualityTarget = 0.30;
    profile.harmonicDegreeDeltaTarget = 0.05;
    profile.confidence = 1.0;
    profile.valid = true;

    return profile;
}

void testEnabledBridgeIsValid()
{
    const auto bridge =
        buildCompositionAIEngineBridge(
            true);

    expect(
        bridge.isValid(),
        "enabled AI engine bridge is valid");
}

void testDisabledBridgeIsInvalid()
{
    const auto bridge =
        buildCompositionAIEngineBridge(
            false);

    expect(
        !bridge.isValid(),
        "disabled AI engine bridge is invalid");
}

void testGuidancePreservesAllTargets()
{
    const auto bridge =
        buildCompositionAIEngineBridge();

    const auto profile =
        buildProfile();

    const auto guidance =
        bridge.deriveGuidance(
            profile);

    expect(
        guidance.isValid(),
        "derived AI guidance is valid");

    expect(
        guidance.roleTarget ==
            profile.roleTarget &&
        guidance.tensionTarget ==
            profile.tensionTarget &&
        guidance.tensionDeltaTarget ==
            profile.tensionDeltaTarget &&
        guidance.harmonyDegreeTarget ==
            profile.harmonyDegreeTarget &&
        guidance.harmonyQualityTarget ==
            profile.harmonyQualityTarget &&
        guidance.harmonicDegreeDeltaTarget ==
            profile.harmonicDegreeDeltaTarget,
        "bridge preserves all musical targets");
}

void testConfidenceIsPreserved()
{
    const auto bridge =
        buildCompositionAIEngineBridge();

    auto profile =
        buildProfile();

    profile.confidence =
        0.73;

    const auto guidance =
        bridge.deriveGuidance(
            profile);

    expect(
        guidance.isValid(),
        "guidance with non-max confidence is valid");

    expect(
        guidance.confidence ==
            0.73,
        "bridge preserves confidence");
}

void testInvalidProfileIsRejected()
{
    const auto bridge =
        buildCompositionAIEngineBridge();

    auto profile =
        buildProfile();

    profile.tensionTarget =
        2.0;

    const auto guidance =
        bridge.deriveGuidance(
            profile);

    expect(
        !guidance.valid,
        "invalid AI profile is rejected by bridge");
}

void testDisabledBridgeProducesNoGuidance()
{
    const auto bridge =
        buildCompositionAIEngineBridge(
            false);

    const auto guidance =
        bridge.deriveGuidance(
            buildProfile());

    expect(
        !guidance.valid,
        "disabled bridge produces no guidance");
}

void testGuidanceIsDeterministic()
{
    const auto bridge =
        buildCompositionAIEngineBridge();

    const auto profile =
        buildProfile();

    const auto first =
        bridge.deriveGuidance(
            profile);

    const auto second =
        bridge.deriveGuidance(
            profile);

    expect(
        first.roleTarget ==
            second.roleTarget &&
        first.tensionTarget ==
            second.tensionTarget &&
        first.harmonyDegreeTarget ==
            second.harmonyDegreeTarget &&
        first.confidence ==
            second.confidence,
        "AI guidance derivation is deterministic");
}

} // namespace

int main()
{
    testEnabledBridgeIsValid();
    testDisabledBridgeIsInvalid();
    testGuidancePreservesAllTargets();
    testConfidenceIsPreserved();
    testInvalidProfileIsRejected();
    testDisabledBridgeProducesNoGuidance();
    testGuidanceIsDeterministic();

    std::cout
        << "MIDI-GenGX AI Engine Bridge tests passed.\n";

    return 0;
}
