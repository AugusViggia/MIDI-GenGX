#pragma once

#include "CompositionMidiCorpusTrainingService.h"
#include "CompositionModelEvaluation.h"
#include "CompositionMusicalEvaluation.h"

namespace midigengx::music
{

struct CompositionMidiTrainingEvaluationResult
{
    CompositionMidiCorpusTrainingResult training;

    CompositionModelEvaluationResult neuralValidation;
    CompositionModelEvaluationResult neuralTest;

    CompositionModelEvaluationResult baselineValidation;
    CompositionModelEvaluationResult baselineTest;

    CompositionMusicalEvaluationResult musicalValidation;
    CompositionMusicalEvaluationResult musicalTest;

    bool valid = false;

    bool isValid() const noexcept;

    bool neuralBeatsBaselineOnTest() const noexcept;
};

CompositionMidiTrainingEvaluationResult
trainAndEvaluateCompositionNeuralModelFromMidiCorpus(
    const std::string& directoryPath,
    bool recursive,
    double validationRatio,
    double testRatio,
    const CompositionNeuralTrainingConfig& config) noexcept;

} // namespace midigengx::music
