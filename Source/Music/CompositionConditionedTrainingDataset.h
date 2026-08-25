#pragma once

#include "CompositionConditionedTrainingSample.h"
#include "CompositionConditioningVocabulary.h"
#include "CompositionSequenceMetadataCatalog.h"

#include <vector>

namespace midigengx::music
{

struct CompositionConditionedTrainingDataset
{
    static constexpr int version = 1;

    CompositionConditioningVocabulary vocabulary;
    std::vector<CompositionConditionedTrainingSample> samples;

    bool verified = false;
    bool valid = false;

    bool isValid() const noexcept;

    std::size_t sampleCount() const noexcept;
};

CompositionConditionedTrainingDataset
buildCompositionConditionedTrainingDataset(
    const std::vector<CompositionMidiTrainingSequence>& sequences,
    const CompositionSequenceMetadataCatalog& metadataCatalog) noexcept;

} // namespace midigengx::music
