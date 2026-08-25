#pragma once

#include "CompositionDataset.h"
#include "CompositionMidiCorpusRecord.h"

#include <vector>

namespace midigengx::music
{

struct CompositionMidiDatasetBuildResult
{
    CompositionDataset dataset;

    std::size_t inputCount = 0;
    std::size_t acceptedCount = 0;
    std::size_t rejectedCount = 0;

    bool valid = false;

    bool isValid() const noexcept;
};

CompositionMidiDatasetBuildResult
buildCompositionDatasetFromMidiCorpus(
    const std::vector<CompositionMidiCorpusRecord>& records) noexcept;

} // namespace midigengx::music
