#include "PhraseTensionArc.h"

#include <algorithm>
#include <cmath>

namespace midigengx::music
{
namespace
{

constexpr double kPi =
    3.14159265358979323846;

int clampPercent(int value) noexcept
{
    return std::clamp(value, 0, 100);
}

double contourPeak(
    midigengx::domain::PhraseContour contour) noexcept
{
    switch (contour)
    {
        case midigengx::domain::PhraseContour::Arch:
            return 0.50;

        case midigengx::domain::PhraseContour::Ascending:
            return 1.00;

        case midigengx::domain::PhraseContour::Descending:
            return 0.00;

        case midigengx::domain::PhraseContour::Flat:
            return 0.50;

        case midigengx::domain::PhraseContour::Valley:
            return 0.50;

        case midigengx::domain::PhraseContour::Custom:
            return 0.50;
    }

    return 0.50;
}

double contourShape(
    double progress,
    midigengx::domain::PhraseContour contour) noexcept
{
    progress =
        std::clamp(progress, 0.0, 1.0);

    switch (contour)
    {
        case midigengx::domain::PhraseContour::Arch:
            return std::sin(
                progress * kPi);

        case midigengx::domain::PhraseContour::Ascending:
            return progress;

        case midigengx::domain::PhraseContour::Descending:
            return 1.0 - progress;

        case midigengx::domain::PhraseContour::Flat:
            return 0.5;

        case midigengx::domain::PhraseContour::Valley:
            return 1.0 -
                   std::sin(
                       progress * kPi);

        case midigengx::domain::PhraseContour::Custom:
            return progress;
    }

    return progress;
}

} // namespace

double PhraseTensionArc::normalizedProgress(
    double beat,
    double totalLengthBeats) noexcept
{
    if (totalLengthBeats <= 0.0)
        return 0.0;

    return std::clamp(
        beat / totalLengthBeats,
        0.0,
        1.0);
}

double PhraseTensionArc::climaxPosition(
    midigengx::domain::PhraseContour contour) noexcept
{
    return contourPeak(contour);
}

int PhraseTensionArc::tensionAtProgress(
    double progress,
    midigengx::domain::PhraseContour contour,
    int baseTension,
    int cadenceStrength) noexcept
{
    baseTension =
        clampPercent(baseTension);

    cadenceStrength =
        clampPercent(cadenceStrength);

    progress =
        std::clamp(
            progress,
            0.0,
            1.0);

    const double shape =
        contourShape(
            progress,
            contour);

    // Cadence strength affects only the approach to the final point. It is
    // deliberately zero at the phrase start and strongest at the endpoint.
    const double cadenceRamp =
        progress * progress;

    const double weighted =
        static_cast<double>(baseTension) +
        shape *
            static_cast<double>(
                100 - baseTension) *
            0.55 +
        cadenceRamp *
            static_cast<double>(
                cadenceStrength) *
            0.35;

    return clampPercent(
        static_cast<int>(
            std::lround(weighted)));
}

int PhraseTensionArc::sectionTension(
    double startBeat,
    double endBeat,
    double totalLengthBeats,
    midigengx::domain::PhraseContour contour,
    int baseTension,
    int cadenceStrength,
    bool cadenceSection) noexcept
{
    const double midpoint =
        (startBeat + endBeat) * 0.5;

    int tension =
        tensionAtProgress(
            normalizedProgress(
                midpoint,
                totalLengthBeats),
            contour,
            baseTension,
            cadenceStrength);

    if (cadenceSection)
    {
        tension =
            std::max(
                tension,
                clampPercent(
                    baseTension +
                    cadenceStrength / 3));
    }

    return clampPercent(tension);
}

} // namespace midigengx::music
