#pragma once

#include "CompositionMidiCorpusDirectoryLoader.h"
#include "CompositionMidiDatasetBuilder.h"
#include "CompositionDatasetManifest.h"
#include "CompositionDatasetPartition.h"
#include "CompositionDatasetPreparedView.h"
#include "CompositionDatasetQuality.h"
#include "CompositionTrainingCorpusArtifact.h"

#include <cstddef>
#include <string>

namespace midigengx::music
{

struct CompositionMidiTrainingPipelineResult
{
    CompositionMidiCorpusDirectoryLoadResult corpus;
    CompositionMidiDatasetBuildResult datasetBuild;
    CompositionDatasetQuality quality;
    CompositionDatasetPartition partition;
    CompositionDatasetManifest manifest;
    CompositionDatasetPreparedView prepared;
    CompositionTrainingCorpusArtifact artifact;

    bool valid = false;

    bool isValid() const noexcept;

    std::size_t sampleCount() const noexcept;
};

CompositionMidiTrainingPipelineResult
buildCompositionMidiTrainingPipeline(
    const std::string& directoryPath,
    bool recursive,
    double validationRatio,
    double testRatio) noexcept;

} // namespace midigengx::music
