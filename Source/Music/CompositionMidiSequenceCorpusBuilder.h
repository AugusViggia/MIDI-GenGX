#pragma once

#include "CompositionMidiTrainingCorpusArtifact.h"

#include <cstddef>
#include <string>
#include <vector>

namespace midigengx::music
{

struct CompositionMidiSequenceCorpusBuildResult
{
    CompositionMidiTrainingCorpusArtifact artifact;

    std::size_t inputFileCount = 0;
    std::size_t sequenceCount = 0;
    std::size_t rejectedCount = 0;
    std::size_t eventCount = 0;

    bool valid = false;

    bool isValid() const noexcept;
};

CompositionMidiSequenceCorpusBuildResult
buildCompositionMidiSequenceCorpusFromDirectory(
    const std::string& directoryPath,
    bool recursive) noexcept;

} // namespace midigengx::music
