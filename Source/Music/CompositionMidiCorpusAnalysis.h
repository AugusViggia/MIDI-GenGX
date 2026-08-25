#pragma once

#include "CompositionMidiCorpusRecord.h"

namespace midigengx::music
{

struct CompositionMidiCorpusAnalysis
{
    static constexpr int version = 1;

    double notesPerBeat = 0.0;
    double averagePitch = 0.0;
    double pitchRange = 0.0;
    double averageVelocity = 0.0;
    double averageDurationBeats = 0.0;
    double maxPolyphony = 0.0;

    bool valid = false;

    bool isValid() const noexcept;
};

CompositionMidiCorpusAnalysis analyzeCompositionMidiCorpus(
    const CompositionMidiCorpusRecord& record) noexcept;

} // namespace midigengx::music
