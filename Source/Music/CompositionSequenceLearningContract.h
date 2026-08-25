#pragma once

#include "CompositionMidiTrainingSequence.h"

#include <cstddef>

namespace midigengx::music
{

enum class CompositionSequenceLearningObjective
{
    NextEventPrediction,
    EventAttributeReconstruction
};

struct CompositionSequenceLearningContract
{
    static constexpr int version = 1;

    CompositionSequenceLearningObjective objective =
        CompositionSequenceLearningObjective::NextEventPrediction;

    std::size_t inputFeatureWidth =
        CompositionMidiTrainingEvent::featureCount;

    std::size_t targetFeatureWidth =
        CompositionMidiTrainingEvent::featureCount;

    std::size_t contextLength = 64;

    bool usesPaddingMask = true;
    bool usesStartToken = false;
    bool usesEndToken = false;
    bool analysisValid = false;

    bool isValid() const noexcept;
};

CompositionSequenceLearningContract
buildCompositionSequenceLearningContract(
    std::size_t contextLength = 64,
    CompositionSequenceLearningObjective objective =
        CompositionSequenceLearningObjective::NextEventPrediction) noexcept;

} // namespace midigengx::music
