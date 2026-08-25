#pragma once

#include "CompositionConditionedTrainingDataset.h"
#include "CompositionMidiTrainingSequence.h"

#include <cstddef>
#include <string>
#include <vector>

namespace midigengx::music
{

struct CompositionRealComposerCorpusPreparationResult
{
    std::vector<CompositionMidiTrainingSequence> sequences;
    CompositionSequenceMetadataCatalog metadataCatalog;
    CompositionConditionedTrainingDataset conditionedDataset;

    std::size_t inputSampleCount = 0;
    std::size_t acceptedSampleCount = 0;
    std::size_t rejectedSampleCount = 0;

    bool valid = false;

    bool isValid() const noexcept;
};

CompositionRealComposerCorpusPreparationResult
prepareRealComposerCorpusFromRecords(
    const std::vector<CompositionMidiCorpusRecord>& records,
    const CompositionSequenceMetadataCatalog& metadataCatalog) noexcept;

CompositionRealComposerCorpusPreparationResult
prepareRealComposerCorpusFromDirectory(
    const std::string& directoryPath,
    bool recursive,
    const CompositionSequenceMetadataCatalog& metadataCatalog) noexcept;

} // namespace midigengx::music
