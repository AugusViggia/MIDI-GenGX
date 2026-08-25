#include "MotifRelationship.h"

#include "MotifDevelopment.h"

#include <algorithm>
#include <cmath>

namespace midigengx::music
{
namespace
{

constexpr double kEpsilon = 1.0e-9;

bool sameRhythm(
    const MotifFingerprint& a,
    const MotifFingerprint& b) noexcept
{
    return a.rhythmSignature ==
           b.rhythmSignature;
}

bool sameContour(
    const MotifFingerprint& a,
    const MotifFingerprint& b) noexcept
{
    return a.contourSignature ==
           b.contourSignature;
}

bool sameIntervals(
    const MotifFingerprint& a,
    const MotifFingerprint& b) noexcept
{
    return a.intervalSignature ==
           b.intervalSignature;
}

bool sameLength(
    const Motif& a,
    const Motif& b) noexcept
{
    return std::abs(
               a.lengthBeats -
               b.lengthBeats) <=
           kEpsilon;
}

bool exactMotifEquivalent(
    const Motif& a,
    const Motif& b) noexcept
{
    if (!sameLength(a, b) ||
        a.notes.size() != b.notes.size())
    {
        return false;
    }

    for (std::size_t i = 0;
         i < a.notes.size();
         ++i)
    {
        const auto& source =
            a.notes[i];

        const auto& candidate =
            b.notes[i];

        if (std::abs(source.startBeat -
                     candidate.startBeat) > kEpsilon ||
            std::abs(source.durationBeats -
                     candidate.durationBeats) > kEpsilon ||
            source.relativePitch != candidate.relativePitch ||
            source.velocityDelta != candidate.velocityDelta)
        {
            return false;
        }
    }

    return true;
}

bool pitchShiftEquivalent(
    const Motif& source,
    const Motif& candidate,
    int& shift) noexcept
{
    if (source.notes.size() !=
        candidate.notes.size() ||
        !sameLength(source, candidate))
    {
        return false;
    }

    if (source.notes.empty())
        return false;

    shift =
        candidate.notes.front().relativePitch -
        source.notes.front().relativePitch;

    for (std::size_t i = 1;
         i < source.notes.size();
         ++i)
    {
        if (candidate.notes[i].relativePitch -
                source.notes[i].relativePitch !=
            shift)
        {
            return false;
        }

        if (std::abs(
                candidate.notes[i].startBeat -
                source.notes[i].startBeat) >
            kEpsilon ||
            std::abs(
                candidate.notes[i].durationBeats -
                source.notes[i].durationBeats) >
            kEpsilon)
        {
            return false;
        }
    }

    return true;
}

bool retrogradeEquivalent(
    const Motif& source,
    const Motif& candidate) noexcept
{
    const auto expected =
        MotifDevelopment::retrograde(
            source);

    if (!expected.isValid() ||
        expected.notes.size() !=
            candidate.notes.size())
    {
        return false;
    }

    for (std::size_t i = 0;
         i < expected.notes.size();
         ++i)
    {
        const auto& a =
            expected.notes[i];
        const auto& b =
            candidate.notes[i];

        if (std::abs(
                a.startBeat -
                b.startBeat) > kEpsilon ||
            std::abs(
                a.durationBeats -
                b.durationBeats) > kEpsilon ||
            a.relativePitch !=
                b.relativePitch)
        {
            return false;
        }
    }

    return true;
}

bool inversionEquivalent(
    const Motif& source,
    const Motif& candidate) noexcept
{
    const auto expected =
        MotifDevelopment::invert(
            source);

    if (!expected.isValid())
        return false;

    if (expected.notes.size() !=
        candidate.notes.size())
    {
        return false;
    }

    for (std::size_t i = 0;
         i < expected.notes.size();
         ++i)
    {
        const auto& a =
            expected.notes[i];

        const auto& b =
            candidate.notes[i];

        if (std::abs(
                a.startBeat -
                b.startBeat) > kEpsilon ||
            std::abs(
                a.durationBeats -
                b.durationBeats) > kEpsilon ||
            a.relativePitch !=
                b.relativePitch)
        {
            return false;
        }
    }

    return true;
}

} // namespace

bool MotifRelationship::isValid() const noexcept
{
    return kind !=
               MotifRelationshipKind::Invalid &&
           confidence >= 0.0 &&
           confidence <= 1.0;
}

MotifRelationship analyzeMotifRelationship(
    const Motif& source,
    const Motif& candidate) noexcept
{
    MotifRelationship result;

    if (!source.isValid() ||
        !candidate.isValid())
    {
        return result;
    }

    const auto sourceFp =
        fingerprintMotif(source);

    const auto candidateFp =
        fingerprintMotif(candidate);

    if (!sourceFp.isValid() ||
        !candidateFp.isValid())
    {
        return result;
    }

    // Canonical identity intentionally ignores absolute pitch. Exact
    // recurrence therefore has to be distinguished from transposition.
    if (exactMotifEquivalent(source, candidate))
    {
        result.kind =
            MotifRelationshipKind::Identity;
        result.confidence = 1.0;
        return result;
    }

    int shift = 0;

    if (pitchShiftEquivalent(
            source,
            candidate,
            shift))
    {
        result.kind =
            MotifRelationshipKind::Transposition;
        result.confidence = 1.0;
        result.transpositionSemitones = shift;
        return result;
    }

    if (retrogradeEquivalent(
            source,
            candidate))
    {
        result.kind =
            MotifRelationshipKind::Retrograde;
        result.confidence = 1.0;
        return result;
    }

    if (inversionEquivalent(
            source,
            candidate))
    {
        result.kind =
            MotifRelationshipKind::Inversion;
        result.confidence = 1.0;
        return result;
    }

    const bool rhythmSame =
        sameRhythm(
            sourceFp,
            candidateFp);

    const bool contourSame =
        sameContour(
            sourceFp,
            candidateFp);

    const bool intervalsSame =
        sameIntervals(
            sourceFp,
            candidateFp);

    if (intervalsSame &&
        contourSame &&
        !rhythmSame)
    {
        result.kind =
            MotifRelationshipKind::RhythmicVariation;
        result.confidence = 0.95;
        return result;
    }

    if (!intervalsSame &&
        contourSame &&
        rhythmSame)
    {
        result.kind =
            MotifRelationshipKind::IntervalVariation;
        result.confidence = 0.90;
        return result;
    }

    if (contourSame ||
        rhythmSame)
    {
        result.kind =
            MotifRelationshipKind::CompoundVariation;
        result.confidence =
            (static_cast<double>(
                 contourSame) +
             static_cast<double>(
                 rhythmSame)) /
            2.0;
        return result;
    }

    return result;
}

} // namespace midigengx::music
