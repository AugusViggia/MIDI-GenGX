#pragma once

#include "CompositionMidiTrainingSequence.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace midigengx::music
{

struct CompositionMidiTrainingCorpusArtifact
{
    static constexpr std::uint32_t magic = 0x4D47534Du; // "MGSM"
    static constexpr std::uint32_t version = 1;

    std::vector<std::uint8_t> bytes;

    bool isValid() const noexcept;
};

CompositionMidiTrainingCorpusArtifact
serializeCompositionMidiTrainingSequences(
    const std::vector<CompositionMidiTrainingSequence>& sequences) noexcept;

bool deserializeCompositionMidiTrainingSequences(
    const CompositionMidiTrainingCorpusArtifact& artifact,
    std::vector<CompositionMidiTrainingSequence>& sequences) noexcept;

} // namespace midigengx::music
