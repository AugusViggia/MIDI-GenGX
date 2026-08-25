#include "MelodicContour.h"

#include <algorithm>
#include <cmath>

namespace midigengx::music
{

double MelodicContour::registerShape(
    midigengx::domain::PhraseContour contour,
    double progress) noexcept
{
    progress =
        std::clamp(
            progress,
            0.0,
            1.0);

    constexpr double pi =
        3.14159265358979323846;

    switch (contour)
    {
        case midigengx::domain::PhraseContour::Arch:
            return 0.5 -
                   0.5 *
                       std::cos(
                           progress *
                           2.0 *
                           pi);

        case midigengx::domain::PhraseContour::Ascending:
            return progress;

        case midigengx::domain::PhraseContour::Descending:
            return 1.0 - progress;

        case midigengx::domain::PhraseContour::Flat:
            return 0.5;

        case midigengx::domain::PhraseContour::Valley:
            return 0.5 +
                   0.5 *
                       std::cos(
                           progress *
                           2.0 *
                           pi);

        case midigengx::domain::PhraseContour::Custom:
            return progress;
    }

    return progress;
}

int MelodicContour::endpointRegisterIndex(
    midigengx::domain::PhraseContour contour,
    std::size_t noteIndex,
    std::size_t noteCount,
    int lowIndex,
    int highIndex) noexcept
{
    lowIndex =
        std::min(
            lowIndex,
            highIndex);

    highIndex =
        std::max(
            lowIndex,
            highIndex);

    if (noteCount == 0)
        return std::clamp(
            lowIndex,
            0,
            highIndex);

    const std::size_t lastIndex =
        noteCount - 1;

    const std::size_t middleIndex =
        noteCount / 2;

    switch (contour)
    {
        case midigengx::domain::PhraseContour::Arch:
            if (noteIndex == 0 ||
                noteIndex == lastIndex)
            {
                return lowIndex;
            }

            if (noteCount > 2 &&
                noteIndex == middleIndex)
            {
                return highIndex;
            }
            break;

        case midigengx::domain::PhraseContour::Ascending:
            if (noteIndex == 0)
                return lowIndex;

            if (noteIndex == lastIndex)
                return highIndex;
            break;

        case midigengx::domain::PhraseContour::Descending:
            if (noteIndex == 0)
                return highIndex;

            if (noteIndex == lastIndex)
                return lowIndex;
            break;

        case midigengx::domain::PhraseContour::Flat:
            if (noteIndex == 0 ||
                noteIndex == lastIndex)
            {
                return (
                    lowIndex +
                    highIndex) / 2;
            }
            break;

        case midigengx::domain::PhraseContour::Valley:
            if (noteIndex == 0 ||
                noteIndex == lastIndex)
            {
                return highIndex;
            }

            if (noteCount > 2 &&
                noteIndex == middleIndex)
            {
                return lowIndex;
            }
            break;

        case midigengx::domain::PhraseContour::Custom:
            break;
    }

    return -1;
}

} // namespace midigengx::music
