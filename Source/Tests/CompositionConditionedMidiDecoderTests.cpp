#include "Music/CompositionConditionedMidiDecoder.h"

#include <cstdlib>
#include <iostream>
#include <vector>

using namespace midigengx::music;

namespace
{

void expect(
    bool condition,
    const char* message)
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

std::vector<double> validFeatures()
{
    return std::vector<double>(
        CompositionMidiTrainingEvent::featureCount,
        0.0);
}

CompositionMidiTrainingEvent previousEvent()
{
    CompositionMidiTrainingEvent event;
    event.features =
        validFeatures();
    return event;
}

void testConfig()
{
    CompositionConditionedMidiDecoderConfig config;

    expect(
        config.isValid(),
        "default decoder config is valid");

    config.defaultChannel = 17;

    expect(
        !config.isValid(),
        "invalid channel is rejected");
}

void testDecodeProducesValidNote()
{
    auto features = validFeatures();
    features[0] = 0.0;  // center -> MIDI ~64
    features[1] = 0.0;  // center velocity
    features[2] = 0.0;  // positive minimum-clamped duration
    features[3] = 0.25; // positive delta
    features[19] = 0.0;

    auto note =
        CompositionConditionedMidiDecoder::decodeEvent(
            features,
            previousEvent(),
            4.0);

    expect(
        note.midiNote >= 0 &&
        note.midiNote <= 127,
        "decoded pitch is in MIDI range");

    expect(
        note.velocity >= 1 &&
        note.velocity <= 127,
        "decoded velocity is valid");

    expect(
        note.startBeat >= 4.0,
        "decoded event does not move backward");

    expect(
        note.durationBeats > 0.0,
        "decoded duration is positive");

    expect(
        note.channel >= 1 &&
        note.channel <= 16,
        "decoded channel is valid");

    note.clamp();

    expect(
        note.startBeat >= 0.0 &&
        note.durationBeats > 0.0,
        "clamped note remains valid");
}

void testForwardRollout()
{
    const auto previous =
        previousEvent();

    double startBeat = 0.0;

    for (int index = 0;
         index < 32;
         ++index)
    {
        auto features =
            validFeatures();

        features[0] =
            -1.0 +
            static_cast<double>(index % 8) /
            4.0;

        features[1] = 0.0;
        features[2] = -0.5;
        features[3] = 0.1;
        features[19] = 0.0;

        const auto note =
            CompositionConditionedMidiDecoder::decodeEvent(
                features,
                previous,
                startBeat);

        expect(
            note.midiNote >= 0 &&
            note.midiNote <= 127 &&
            note.velocity >= 1 &&
            note.velocity <= 127 &&
            note.startBeat >= 0.0 &&
            note.durationBeats > 0.0 &&
            note.channel >= 1 &&
            note.channel <= 16,
            "rollout note is valid");

        expect(
            note.startBeat >= startBeat,
            "rollout remains monotonic");

        startBeat =
            note.startBeat;
    }
}

} // namespace

int main()
{
    testConfig();
    testDecodeProducesValidNote();
    testForwardRollout();

    std::cout
        << "MIDI-GenGX Phase 120.4A conditioned MIDI decoder tests passed.\n";

    return 0;
}
