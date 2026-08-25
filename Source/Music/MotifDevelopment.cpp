#include "MotifDevelopment.h"

#include <algorithm>
#include <cmath>

namespace midigengx::music
{
namespace
{

constexpr double kEpsilon = 1.0e-9;

Motif normalizedCopy(const Motif& motif) noexcept
{
    Motif result = motif;

    std::sort(
        result.notes.begin(),
        result.notes.end(),
        [](const MotifNote& a, const MotifNote& b)
        {
            if (a.startBeat == b.startBeat)
                return a.relativePitch < b.relativePitch;

            return a.startBeat < b.startBeat;
        });

    return result;
}

} // namespace

Motif MotifDevelopment::transpose(
    const Motif& motif,
    int semitones) noexcept
{
    if (!motif.isValid())
        return {};

    Motif result = normalizedCopy(motif);

    for (auto& note : result.notes)
        note.relativePitch += semitones;

    return result;
}

Motif MotifDevelopment::invert(
    const Motif& motif) noexcept
{
    if (!motif.isValid())
        return {};

    Motif result = normalizedCopy(motif);

    for (auto& note : result.notes)
        note.relativePitch = -note.relativePitch;

    return result;
}

Motif MotifDevelopment::retrograde(
    const Motif& motif) noexcept
{
    if (!motif.isValid())
        return {};

    Motif result = normalizedCopy(motif);

    const double length =
        result.lengthBeats;

    for (auto& note : result.notes)
    {
        const double originalEnd =
            note.startBeat +
            note.durationBeats;

        note.startBeat =
            std::max(
                0.0,
                length -
                    originalEnd);
    }

    std::sort(
        result.notes.begin(),
        result.notes.end(),
        [](const MotifNote& a, const MotifNote& b)
        {
            if (std::abs(a.startBeat - b.startBeat) < 1.0e-9)
                return a.relativePitch < b.relativePitch;

            return a.startBeat < b.startBeat;
        });

    return result;
}

Motif MotifDevelopment::varyIntervals(
    const Motif& motif,
    int variationAmount) noexcept
{
    if (!motif.isValid())
        return {};

    variationAmount =
        std::clamp(
            variationAmount,
            0,
            100);

    Motif result =
        normalizedCopy(motif);

    if (result.notes.size() < 2 ||
        variationAmount == 0)
    {
        return result;
    }

    // The first note anchors motif identity. Later notes receive bounded,
    // deterministic interval perturbations. The perturbation follows the
    // original interval direction so contour identity is retained.
    for (std::size_t index = 1;
         index < result.notes.size();
         ++index)
    {
        const int previous =
            result.notes[index - 1].relativePitch;

        const int current =
            result.notes[index].relativePitch;

        const int interval =
            current - previous;

        if (interval == 0)
            continue;

        const int magnitude =
            std::max(
                1,
                std::min(
                    2,
                    variationAmount / 50));

        const int direction =
            interval > 0
                ? 1
                : -1;

        const bool selected =
            ((index * 37u +
              static_cast<std::size_t>(
                  variationAmount)) %
             100u) <
            static_cast<std::size_t>(
                variationAmount);

        if (!selected)
            continue;

        result.notes[index].relativePitch +=
            direction * magnitude;
    }

    return result;
}

Motif MotifDevelopment::stretchTime(
    const Motif& motif,
    double factor) noexcept
{
    if (!motif.isValid() ||
        !std::isfinite(factor) ||
        factor <= kEpsilon)
    {
        return {};
    }

    Motif result = normalizedCopy(motif);

    for (auto& note : result.notes)
    {
        note.startBeat *= factor;
        note.durationBeats *= factor;
    }

    result.lengthBeats *= factor;
    return result;
}

Motif MotifDevelopment::repeat(
    const Motif& motif,
    int repetitions,
    double gapBeats) noexcept
{
    if (!motif.isValid() ||
        repetitions <= 0 ||
        !std::isfinite(gapBeats) ||
        gapBeats < 0.0)
    {
        return {};
    }

    Motif source = normalizedCopy(motif);
    Motif result;

    result.notes.reserve(
        source.notes.size() *
        static_cast<std::size_t>(repetitions));

    const double period =
        source.lengthBeats + gapBeats;

    for (int repetitionIndex = 0;
         repetitionIndex < repetitions;
         ++repetitionIndex)
    {
        const double offset =
            static_cast<double>(repetitionIndex) *
            period;

        for (const auto& sourceNote : source.notes)
        {
            auto note = sourceNote;
            note.startBeat += offset;
            result.notes.push_back(note);
        }
    }

    result.lengthBeats =
        source.lengthBeats *
            static_cast<double>(repetitions) +
        gapBeats *
            static_cast<double>(
                std::max(0, repetitions - 1));

    return result;
}

Motif MotifDevelopment::sequence(
    const Motif& motif,
    int repetitions,
    int semitonesPerRepetition) noexcept
{
    if (!motif.isValid() ||
        repetitions <= 0 ||
        repetitions > 8)
    {
        return {};
    }

    const Motif source =
        normalizedCopy(motif);

    Motif result;

    result.notes.reserve(
        source.notes.size() *
        static_cast<std::size_t>(
            repetitions));

    const double period =
        source.lengthBeats;

    for (int repetitionIndex = 0;
         repetitionIndex < repetitions;
         ++repetitionIndex)
    {
        const int transposition =
            repetitionIndex *
            semitonesPerRepetition;

        const double offset =
            static_cast<double>(
                repetitionIndex) *
            period;

        for (const auto& sourceNote : source.notes)
        {
            auto note = sourceNote;
            note.startBeat += offset;
            note.relativePitch += transposition;
            result.notes.push_back(note);
        }
    }

    result.lengthBeats =
        source.lengthBeats *
        static_cast<double>(repetitions);

    return result;
}


Motif MotifDevelopment::transposeAndStretch(
    const Motif& motif,
    int semitones,
    double timeFactor) noexcept
{
    const auto transposed =
        transpose(motif, semitones);

    if (!transposed.isValid())
        return {};

    return stretchTime(
        transposed,
        timeFactor);
}

} // namespace midigengx::music
