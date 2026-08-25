#include "RhythmPlan.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace midigengx::music
{
namespace
{

constexpr double kEpsilon = 1.0e-9;

double durationForNoteLength(
    midigengx::domain::NoteLength noteLength,
    double originalDuration) noexcept
{
    switch (noteLength)
    {
        case midigengx::domain::NoteLength::Short:
            return 0.25;

        case midigengx::domain::NoteLength::Medium:
            return 0.5;

        case midigengx::domain::NoteLength::Long:
            return 1.5;

        case midigengx::domain::NoteLength::Legato:
            return 2.0;

        case midigengx::domain::NoteLength::Staccato:
            return 0.2;

        case midigengx::domain::NoteLength::Auto:
            return std::clamp(
                originalDuration,
                0.25,
                2.0);

        case midigengx::domain::NoteLength::Custom:
            return std::max(
                0.125,
                originalDuration);
    }

    return std::max(0.125, originalDuration);
}

std::vector<double> onsetsForFeel(
    midigengx::domain::RhythmFeel feel) noexcept
{
    switch (feel)
    {
        case midigengx::domain::RhythmFeel::Straight:
            return {0.0, 1.0, 2.0, 3.0};

        case midigengx::domain::RhythmFeel::Syncopated:
            return {0.0, 0.75, 1.5, 2.25, 3.0};

        case midigengx::domain::RhythmFeel::Groovy:
            return {0.0, 0.5, 1.5, 2.0, 3.0};

        case midigengx::domain::RhythmFeel::Driving:
            return {0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5};

        case midigengx::domain::RhythmFeel::Offbeat:
            return {0.5, 1.5, 2.5, 3.5};

        case midigengx::domain::RhythmFeel::Minimal:
            return {0.0, 2.0};

        case midigengx::domain::RhythmFeel::Custom:
            return {0.0, 1.0, 2.0, 3.0};
    }

    return {0.0, 1.0, 2.0, 3.0};
}

std::vector<double> defaultOffbeats() noexcept
{
    return {0.5, 1.5, 2.5, 3.5};
}

double nearestOnset(
    const std::vector<double>& onsets,
    double target) noexcept
{
    if (onsets.empty())
        return 0.0;

    double best = onsets.front();
    double bestDistance =
        std::numeric_limits<double>::max();

    for (const auto onset : onsets)
    {
        const double distance =
            std::abs(onset - target);

        if (distance < bestDistance)
        {
            bestDistance = distance;
            best = onset;
        }
    }

    return best;
}

} // namespace

bool RhythmPlan::isValid() const noexcept
{
    if (barLengthBeats <= 0.0 ||
        gridBeats <= 0.0 ||
        defaultDurationBeats <= 0.0 ||
        density < 0 ||
        density > 100 ||
        syncopation < 0 ||
        syncopation > 100 ||
        primaryOnsets.empty())
    {
        return false;
    }

    return true;
}

RhythmPlan planRhythm(
    const midigengx::domain::MusicalContext& inputContext) noexcept
{
    auto context = inputContext;
    context.normalize();

    RhythmPlan plan;

    plan.density =
        context.parameters.density;

    plan.syncopation =
        context.parameters.syncopation;

    plan.noteLengthVariation =
        context.parameters.noteLengthVariation;

    plan.primaryOnsets =
        onsetsForFeel(
            context.parameters.rhythm);

    plan.offbeatOnsets =
        defaultOffbeats();

    switch (context.parameters.rhythm)
    {
        case midigengx::domain::RhythmFeel::Driving:
            plan.gridBeats = 0.5;
            break;

        case midigengx::domain::RhythmFeel::Syncopated:
        case midigengx::domain::RhythmFeel::Groovy:
            plan.gridBeats = 0.25;
            break;

        default:
            plan.gridBeats = 0.5;
            break;
    }

    plan.defaultDurationBeats =
        durationForNoteLength(
            context.parameters.noteLength,
            plan.gridBeats);

    return plan;
}

Motif applyRhythmPlan(
    const Motif& motif,
    const RhythmPlan& plan) noexcept
{
    if (!motif.isValid() ||
        !plan.isValid())
    {
        return {};
    }

    Motif result;
    result.lengthBeats =
        motif.lengthBeats;

    const int density = plan.density;

    // Density controls how many motif events survive. This is deterministic
    // and keeps at least one event when a valid motif is supplied.
    int keepModulo = 1;

    if (density < 25)
        keepModulo = 4;
    else if (density < 50)
        keepModulo = 3;
    else if (density < 75)
        keepModulo = 2;

    for (std::size_t index = 0;
         index < motif.notes.size();
         ++index)
    {
        if (keepModulo > 1 &&
            index % static_cast<std::size_t>(keepModulo) != 0)
        {
            continue;
        }

        auto note = motif.notes[index];

        const double barOffset =
            std::fmod(
                std::max(0.0, note.startBeat),
                plan.barLengthBeats);

        const double cyclePosition =
            std::fmod(
                barOffset,
                4.0);

        const bool preferOffbeat =
            plan.syncopation >= 60 &&
            (index % 2 == 1);

        double onset = nearestOnset(
            preferOffbeat
                ? plan.offbeatOnsets
                : plan.primaryOnsets,
            cyclePosition);

        onset +=
            std::floor(
                std::max(0.0, note.startBeat) /
                plan.barLengthBeats) *
            plan.barLengthBeats;

        note.startBeat = onset;

        // Note-length variation changes duration in a deterministic
        // alternating pattern, never the musical ordering.
        double duration =
            plan.defaultDurationBeats;

        if (plan.syncopation >= 70 &&
            (index % 3 == 2))
        {
            duration *= 0.75;
        }

        if (plan.noteLengthVariation >= 1 &&
            (index % 4 == 3))
        {
            const double variation =
                1.0 -
                static_cast<double>(
                    plan.noteLengthVariation) *
                0.005;

            duration *=
                std::clamp(
                    variation,
                    0.5,
                    1.0);
        }

        note.durationBeats =
            std::max(
                0.125,
                duration);

        result.notes.push_back(note);
    }

    if (result.notes.empty())
        result.notes.push_back(
            motif.notes.front());

    std::sort(
        result.notes.begin(),
        result.notes.end(),
        [](const MotifNote& a, const MotifNote& b)
        {
            if (a.startBeat == b.startBeat)
                return a.relativePitch < b.relativePitch;

            return a.startBeat < b.startBeat;
        });

    // Keep the motif inside its declared phrase extent.
    for (auto& note : result.notes)
    {
        note.startBeat =
            std::clamp(
                note.startBeat,
                0.0,
                std::max(
                    0.0,
                    result.lengthBeats -
                        0.125));

        note.durationBeats =
            std::min(
                note.durationBeats,
                std::max(
                    0.125,
                    result.lengthBeats -
                        note.startBeat));
    }

    return result;
}

} // namespace midigengx::music
