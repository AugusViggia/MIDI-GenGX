#pragma once

#include "MusicalContext.h"

namespace midigengx::domain
{

struct GenerationIntent
{
    MusicalContext context{};

    int lyrical = 0;
    int energetic = 0;
    int dark = 0;
    int playful = 0;
    int spacious = 0;

    void normalize() noexcept
    {
        context.normalize();

        lyrical = clamp(lyrical);
        energetic = clamp(energetic);
        dark = clamp(dark);
        playful = clamp(playful);
        spacious = clamp(spacious);
    }

private:
    static int clamp(int value) noexcept
    {
        return value < 0
            ? 0
            : (value > 100 ? 100 : value);
    }
};

} // namespace midigengx::domain
