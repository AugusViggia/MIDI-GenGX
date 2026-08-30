#include "CompositionConditionedMidiDecoder.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace midigengx::music
{
namespace
{

constexpr std::size_t kPitch = 0;
constexpr std::size_t kVelocity = 1;
constexpr std::size_t kDuration = 2;
constexpr std::size_t kDelta = 3;
constexpr std::size_t kChannel = 19;

int decodeByteLike(
    double value,
    int maximum) noexcept
{
    const double normalized =
        std::clamp(value, -1.0, 1.0);

    const double mapped =
        (normalized + 1.0) * 0.5 *
        static_cast<double>(maximum);

    return std::clamp(
        static_cast<int>(std::lround(mapped)),
        0,
        maximum);
}

double decodePositive(
    double value,
    double maximum) noexcept
{
    const double normalized =
        std::clamp(value, -1.0, 1.0);

    return std::clamp(
        (normalized + 1.0) * 0.5 * maximum,
        0.0,
        maximum);
}

bool finiteFeatures(
    const std::vector<double>& features) noexcept
{
    for (const auto value : features)
    {
        if (!std::isfinite(value) ||
            value < -1.0 ||
            value > 1.0)
            return false;
    }

    return true;
}

} // namespace

bool CompositionConditionedMidiDecoderConfig::isValid()
    const noexcept
{
    return std::isfinite(minimumDurationBeats) &&
           std::isfinite(maximumDurationBeats) &&
           minimumDurationBeats > 0.0 &&
           maximumDurationBeats >= minimumDurationBeats &&
           defaultChannel >= 1 &&
           defaultChannel <= 16;
}

NoteEvent CompositionConditionedMidiDecoder::decodeEvent(
    const std::vector<double>& features,
    const CompositionMidiTrainingEvent& previousEvent,
    const double previousStartBeat,
    const CompositionConditionedMidiDecoderConfig& config) noexcept
{
    NoteEvent result;

    if (!config.isValid() ||
        features.size() !=
            CompositionMidiTrainingEvent::featureCount ||
        !previousEvent.isValid() ||
        !std::isfinite(previousStartBeat) ||
        previousStartBeat < 0.0 ||
        !finiteFeatures(features))
    {
        return result;
    }

    // Current representation semantics:
    // [0] pitch, [1] velocity, [2] duration, [3] delta, [19] channel.
    // The remaining derived/structural features are intentionally not
    // independently converted into contradictory NoteEvent fields.
    result.midiNote =
        decodeByteLike(features[kPitch], 127);

    result.velocity =
        std::clamp(
            decodeByteLike(features[kVelocity], 127),
            1,
            127);

    const double duration =
        std::clamp(
            decodePositive(
                features[kDuration],
                config.maximumDurationBeats),
            config.minimumDurationBeats,
            config.maximumDurationBeats);

    const double delta =
        decodePositive(
            features[kDelta],
            config.maximumDurationBeats);

    // Keep generated events strictly forward-moving. A normalized delta of
    // -1 therefore becomes zero movement, while non-zero positive deltas
    // advance the sequence. The minimum duration remains authoritative.
    result.startBeat =
        std::max(
            0.0,
            previousStartBeat + delta);

    result.durationBeats =
        duration;

    const int decodedChannel =
        decodeByteLike(features[kChannel], 15) + 1;

    result.channel =
        std::clamp(
            decodedChannel,
            1,
            16);

    // In the absence of a trustworthy absolute channel feature, keep a safe
    // MIDI channel. The current schema normally contains channel information,
    // so decodedChannel is used above; defaultChannel is used only as a final
    // safety fallback.
    if (result.channel < 1 ||
        result.channel > 16)
    {
        result.channel =
            config.defaultChannel;
    }

    result.clamp();
    return result;
}

} // namespace midigengx::music
