#pragma once

#include "CompositionDatasetSample.h"
#include "CompositionMidiCorpusAnalysis.h"
#include "CompositionMidiMotifAnalysis.h"
#include "CompositionMidiHarmony.h"
#include "CompositionMidiSectionAnalyzer.h"

#include <string>

namespace midigengx::music
{

struct CompositionMidiDatasetFeatureExtractor
{
    static constexpr int version = 1;

    CompositionDatasetSample buildSample(
        const CompositionMidiCorpusRecord& record,
        const CompositionMidiCorpusAnalysis& analysis,
        const CompositionMidiSectionAnalysis& sections,
        const CompositionMidiHarmonyAnalysis& harmony,
        const CompositionMidiMotifAnalysis& motifs,
        const std::string& sampleId) const noexcept;
};

} // namespace midigengx::music
