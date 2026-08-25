#pragma once

#include "Motif.h"

namespace midigengx::music
{

struct MotifDevelopment
{
    static Motif transpose(
        const Motif& motif,
        int semitones) noexcept;

    static Motif invert(
        const Motif& motif) noexcept;

    static Motif retrograde(
        const Motif& motif) noexcept;

    static Motif varyIntervals(
        const Motif& motif,
        int variationAmount) noexcept;

    static Motif stretchTime(
        const Motif& motif,
        double factor) noexcept;

    static Motif repeat(
        const Motif& motif,
        int repetitions,
        double gapBeats = 0.0) noexcept;

    static Motif sequence(
        const Motif& motif,
        int repetitions,
        int semitonesPerRepetition) noexcept;

    static Motif transposeAndStretch(
        const Motif& motif,
        int semitones,
        double timeFactor) noexcept;
};

} // namespace midigengx::music
