#pragma once

#include "../Domain/MusicalContext.h"

namespace midigengx::music
{

struct MelodicContour
{
    static double registerShape(
        midigengx::domain::PhraseContour contour,
        double progress) noexcept;

    static int endpointRegisterIndex(
        midigengx::domain::PhraseContour contour,
        std::size_t noteIndex,
        std::size_t noteCount,
        int lowIndex,
        int highIndex) noexcept;
};

} // namespace midigengx::music
