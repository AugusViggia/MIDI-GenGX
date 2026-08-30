#pragma once

#include "CompositionMidiTrainingSequence.h"

#include <cstddef>

namespace midigengx::music
{

struct CompositionConditionedMidiDecoderConfig
{
    double minimumDurationBeats = 0.05;
    double maximumDurationBeats = 16.0;
    int defaultChannel = 1;

    bool isValid() const noexcept;
};

struct CompositionConditionedMidiDecoder
{
    static constexpr int version = 1;

    static NoteEvent decodeEvent(
        const std::vector<double>& features,
        const CompositionMidiTrainingEvent& previousEvent,
        double previousStartBeat,
        const CompositionConditionedMidiDecoderConfig& config = {}) noexcept;
};

} // namespace midigengx::music
