#include "CompositionMidiTrainingPipeline.h"

#include "CompositionMidiCorpusDirectoryLoader.h"
#include "CompositionMidiDatasetBuilder.h"
#include "CompositionDatasetManifest.h"
#include "CompositionDatasetPartition.h"
#include "CompositionDatasetQuality.h"
#include "CompositionDatasetPreparedView.h"

namespace midigengx::music
{

bool CompositionMidiTrainingPipelineResult::isValid()
    const noexcept
{
    return valid &&
           corpus.isValid() &&
           datasetBuild.isValid() &&
           quality.isValid() &&
           partition.isValid(
               datasetBuild.dataset.size()) &&
           manifest.isValid() &&
           prepared.isValid() &&
           artifact.isValid() &&
           datasetBuild.dataset.size() ==
               quality.sampleCount &&
           quality.sampleCount ==
               manifest.sampleCount &&
           manifest.sampleCount ==
               prepared.normalizedBatch.sampleCount;
}

std::size_t
CompositionMidiTrainingPipelineResult::sampleCount()
    const noexcept
{
    return datasetBuild.dataset.size();
}

CompositionMidiTrainingPipelineResult
buildCompositionMidiTrainingPipeline(
    const std::string& directoryPath,
    bool recursive,
    double validationRatio,
    double testRatio)
    noexcept
{
    CompositionMidiTrainingPipelineResult result;

    result.corpus =
        loadCompositionMidiCorpusDirectory(
            directoryPath,
            recursive);

    if (!result.corpus.isValid() ||
        result.corpus.records.empty())
    {
        return result;
    }

    result.datasetBuild =
        buildCompositionDatasetFromMidiCorpus(
            result.corpus.records);

    if (!result.datasetBuild.isValid())
        return result;

    result.quality =
        assessCompositionDatasetQuality(
            result.datasetBuild.dataset);

    if (!result.quality.isValid())
        return result;

    result.partition =
        buildCompositionDatasetPartition(
            result.datasetBuild.dataset,
            validationRatio,
            testRatio);

    if (!result.partition.isValid(
            result.datasetBuild.dataset.size()))
    {
        return result;
    }

    result.manifest =
        buildCompositionDatasetManifest(
            result.datasetBuild.dataset,
            result.quality,
            result.partition);

    if (!result.manifest.isValid())
        return result;

    result.prepared =
        prepareCompositionDatasetForLearning(
            result.datasetBuild.dataset,
            result.quality,
            result.manifest,
            result.partition);

    if (!result.prepared.isValid())
        return result;

    result.artifact =
        serializeCompositionTrainingCorpus(
            result.prepared);

    if (!result.artifact.isValid())
        return result;

    result.valid =
        true;

    if (!result.isValid())
        result.valid = false;

    return result;
}

} // namespace midigengx::music
