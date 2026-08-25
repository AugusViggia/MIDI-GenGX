#pragma once

#include "MotifFingerprint.h"

namespace midigengx::music
{

enum class MotifRelationshipKind
{
    Invalid,
    Identity,
    Transposition,
    Retrograde,
    Inversion,
    RhythmicVariation,
    IntervalVariation,
    CompoundVariation
};

struct MotifRelationship
{
    MotifRelationshipKind kind =
        MotifRelationshipKind::Invalid;

    double confidence = 0.0;
    int transpositionSemitones = 0;

    bool isValid() const noexcept;
};

MotifRelationship analyzeMotifRelationship(
    const Motif& source,
    const Motif& candidate) noexcept;

} // namespace midigengx::music
