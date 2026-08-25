#include "CompositionMidiCorpusRecord.h"

#include <algorithm>

namespace midigengx::music
{

bool CompositionMidiNote::isValid() const noexcept
{
    return channel < 16 &&
           midiNote <= 127 &&
           velocity <= 127 &&
           endTick > startTick;
}

bool CompositionMidiCorpusRecord::isValid() const noexcept
{
    if (!analysisValid ||
        sampleId.empty() ||
        ticksPerQuarterNote == 0 ||
        trackCount == 0 ||
        notes.empty() ||
        lengthTicks == 0)
    {
        return false;
    }

    std::uint32_t maximumEndTick = 0;

    for (const auto& note :
         notes)
    {
        if (!note.isValid())
            return false;

        maximumEndTick =
            std::max(
                maximumEndTick,
                note.endTick);
    }

    return maximumEndTick <= lengthTicks;
}

std::size_t CompositionMidiCorpusRecord::noteCount() const noexcept
{
    return notes.size();
}

} // namespace midigengx::music
