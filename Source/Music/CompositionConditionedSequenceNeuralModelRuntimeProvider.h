#pragma once

#include "CompositionConditionedSequenceNeuralModelRuntimeLoader.h"
#include "CompositionConditionedMidiDecoder.h"
#include "CompositionMidiSequenceWindow.h"
#include "../Domain/MusicalContext.h"
#include "Phrase.h"

#include <cstdint>
#include <vector>

namespace midigengx::music
{

class CompositionConditionedSequenceNeuralModelRuntimeProvider
{
public:
    bool load(
        const CompositionConditionedSequenceNeuralModelArtifact& artifact) noexcept;

    bool isReady() const noexcept;

    void clear() noexcept;

    Phrase generate(
        const midigengx::domain::MusicalContext& context,
        std::uint32_t seed) const noexcept;

private:
    CompositionConditionedSequenceNeuralModel model;
    bool ready = false;
};

} // namespace midigengx::music
