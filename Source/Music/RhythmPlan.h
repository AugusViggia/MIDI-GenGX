#pragma once

#include "../Domain/MusicalContext.h"
#include "Motif.h"

#include <vector>

namespace midigengx::music
{

struct RhythmPlan
{
    double barLengthBeats = 4.0;
    double gridBeats = 0.5;
    double defaultDurationBeats = 0.5;

    std::vector<double> primaryOnsets;
    std::vector<double> offbeatOnsets;

    int density = 50;
    int syncopation = 50;
    int noteLengthVariation = 0;

    bool isValid() const noexcept;
};

RhythmPlan planRhythm(
    const midigengx::domain::MusicalContext& context) noexcept;

Motif applyRhythmPlan(
    const Motif& motif,
    const RhythmPlan& plan) noexcept;

} // namespace midigengx::music
