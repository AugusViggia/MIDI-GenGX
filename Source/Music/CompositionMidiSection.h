#pragma once

#include "CompositionMidiCorpusRecord.h"
#include "PhraseStructure.h"

#include <cstddef>
#include <cstdint>

namespace midigengx::music
{

struct CompositionMidiSection
{
    static constexpr int version = 1;

    std::size_t sectionIndex = 0;

    PhraseSection role =
        PhraseSection::Opening;

    std::uint32_t startTick = 0;
    std::uint32_t endTick = 0;

    double notesPerBeat = 0.0;
    double averagePitch = 0.0;
    double averageVelocity = 0.0;
    double averageDurationBeats = 0.0;
    double maxPolyphony = 0.0;

    int tension = 0;

    bool valid = false;

    bool isValid(
        std::uint16_t ticksPerQuarterNote) const noexcept;
};

} // namespace midigengx::music
