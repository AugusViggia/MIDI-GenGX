#pragma once

#include "CompositionMidiTrainingSequence.h"
#include "CompositionSequenceMetadata.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace midigengx::music
{

struct CompositionConditionedTrainingSample
{
    CompositionMidiTrainingSequence sequence;
    CompositionSequenceMetadata metadata;

    std::uint32_t composerIndex = 0;
    std::uint32_t styleIndex = 0;
    std::uint32_t eraIndex = 0;
    std::uint32_t instrumentationIndex = 0;

    static constexpr std::size_t knowledgeFeatureCount = 37;
    std::array<double, knowledgeFeatureCount> knowledgeFeatures{};

    bool valid = false;

    bool isValid() const noexcept;
};

} // namespace midigengx::music
