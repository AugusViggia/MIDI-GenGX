#include "GenerationActivationPolicy.h"

#include <cmath>

namespace midigengx::generation
{

bool shouldAdoptPublishedPhrase(
    bool hasActivePhrase,
    bool transportPlaying,
    double currentPpq,
    double previousPpq,
    double phraseLengthBeats) noexcept
{
    if (!hasActivePhrase)
        return true;

    if (!transportPlaying)
        return true;

    if (!std::isfinite(currentPpq) ||
        !std::isfinite(previousPpq) ||
        !std::isfinite(phraseLengthBeats) ||
        phraseLengthBeats <= 0.0)
    {
        return false;
    }

    // While playing an existing phrase, never replace it in the middle of a
    // musical cycle. A published phrase becomes active only after crossing
    // a phrase boundary.
    const auto previousCycle =
        static_cast<long long>(
            std::floor(
                previousPpq /
                phraseLengthBeats));

    const auto currentCycle =
        static_cast<long long>(
            std::floor(
                currentPpq /
                phraseLengthBeats));

    return currentCycle > previousCycle;
}

} // namespace midigengx::generation
