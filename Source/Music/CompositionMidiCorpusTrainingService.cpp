#include "CompositionMidiCorpusTrainingService.h"

namespace midigengx::music
{

bool CompositionMidiCorpusTrainingResult::isValid()
    const noexcept
{
    return valid &&
           pipeline.isValid() &&
           neuralTraining.isValid() &&
           sampleCount() ==
               pipeline.prepared.normalizedBatch.sampleCount &&
           neuralTraining.model.isValid();
}

std::size_t
CompositionMidiCorpusTrainingResult::sampleCount()
    const noexcept
{
    return pipeline.sampleCount();
}

CompositionMidiCorpusTrainingResult
trainCompositionNeuralModelFromMidiCorpus(
    const std::string& directoryPath,
    bool recursive,
    double validationRatio,
    double testRatio,
    const CompositionNeuralTrainingConfig& config)
    noexcept
{
    CompositionMidiCorpusTrainingResult result;

    result.pipeline =
        buildCompositionMidiTrainingPipeline(
            directoryPath,
            recursive,
            validationRatio,
            testRatio);

    if (!result.pipeline.isValid())
        return result;

    if (result.pipeline.prepared.trainingCount() == 0)
        return result;

    result.neuralTraining =
        trainCompositionNeuralArtifact(
            result.pipeline.prepared,
            config);

    if (!result.neuralTraining.isValid())
        return result;

    result.valid = true;

    return result;
}

} // namespace midigengx::music
