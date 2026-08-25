#pragma once

#include "Motif.h"
#include "../Domain/MusicalContext.h"

#include <cstdint>

namespace midigengx::music
{

struct MotifPhraseComposer
{
    static Phrase compose(
        const Motif& seedMotif,
        const midigengx::domain::MusicalContext& context,
        std::uint32_t seed) noexcept;
};

} // namespace midigengx::music
