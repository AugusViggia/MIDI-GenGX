#pragma once

#include "CompositionMidiCorpusRecord.h"
#include "CompositionMidiHarmony.h"
#include "CompositionMidiMotifAnalysis.h"
#include "CompositionMidiSectionAnalyzer.h"

#include <cstddef>
#include <string>
#include <vector>

namespace midigengx::music
{

struct CompositionMidiTrainingEvent
{
    // Phase 83 representation is intentionally event-based.
    // Each event contains a fixed-width numeric feature vector so a future
    // sequence model can consume the representation without knowing anything
    // about the MIDI parser or musical domain classes.
    static constexpr std::size_t featureCount = 20;

    std::vector<double> features;

    bool isValid() const noexcept;
};

struct CompositionMidiTrainingSequence
{
    static constexpr int version = 2;

    std::string sampleId;

    std::vector<CompositionMidiTrainingEvent> events;

    std::size_t featureWidth =
        CompositionMidiTrainingEvent::featureCount;

    bool analysisValid = false;

    bool isValid() const noexcept;

    std::size_t eventCount() const noexcept;
};

CompositionMidiTrainingSequence
buildCompositionMidiTrainingSequence(
    const CompositionMidiCorpusRecord& record,
    const CompositionMidiSectionAnalysis& sections,
    const CompositionMidiHarmonyAnalysis& harmony,
    const CompositionMidiMotifAnalysis& motifs) noexcept;

} // namespace midigengx::music
