#include "Music/CompositionNeuralModel.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

using namespace midigengx::music;

namespace
{
void expect(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr
            << "FAILED: "
            << message
            << '\n';
        std::exit(1);
    }
}

CompositionLearningContract buildContract()
{
    CompositionLearningContract contract;

    contract.analysisValid = true;
    contract.objective =
        LearningObjective::NextSectionPrediction;
    contract.globalInputWidth = 13;
    contract.sectionInputWidth = 6;
    contract.targetWidth = 6;
    contract.contextLength = 4;
    contract.usesSectionMask = true;

    return contract;
}

void testDeterministicInitialization()
{
    const auto contract =
        buildContract();

    const auto first =
        initializeCompositionNeuralModel(
            contract);

    const auto second =
        initializeCompositionNeuralModel(
            contract);

    expect(
        first.isValid() &&
        second.isValid(),
        "initialized neural model is valid");

    expect(
        first.inputWeights ==
            second.inputWeights,
        "input weights initialize deterministically");

    expect(
        first.outputWeights ==
            second.outputWeights,
        "output weights initialize deterministically");

    expect(
        first.hiddenBias ==
            second.hiddenBias &&
        first.outputBias ==
            second.outputBias &&
        first.contextResidualWeights ==
            second.contextResidualWeights,
        "biases and residual parameters initialize deterministically");
}

void testParameterShape()
{
    const auto contract =
        buildContract();

    const auto model =
        initializeCompositionNeuralModel(
            contract);

    expect(
        model.inputWeights.size() ==
            (13 + 6) *
            CompositionNeuralModel::hiddenWidth,
        "input weight matrix has correct shape");

    expect(
        model.outputWeights.size() ==
            CompositionNeuralModel::hiddenWidth *
            6,
        "output weight matrix has correct shape");

    expect(
        model.hiddenBias.size() ==
            CompositionNeuralModel::hiddenWidth &&
        model.outputBias.size() == 6,
        "bias vectors have correct shape");
}

void testContextResidualShape()
{
    const auto contract =
        buildContract();

    const auto model =
        initializeCompositionNeuralModel(
            contract);

    expect(
        model.contextResidualWeights.size() ==
            contract.sectionInputWidth *
            contract.targetWidth,
        "context residual matrix has correct shape");

    for (const auto value :
         model.contextResidualWeights)
    {
        expect(
            value == 0.0,
            "context residual starts neutral");
    }
}

void testPredictionShapeAndRange()
{
    const auto contract =
        buildContract();

    const auto model =
        initializeCompositionNeuralModel(
            contract);

    const std::vector<double> global(
        13,
        0.25);

    const std::vector<double> section(
        6,
        -0.10);

    const auto prediction =
        model.predictNextSection(
            global,
            section,
            true);

    expect(
        prediction.isValid(6),
        "neural prediction is valid");

    for (const auto value :
         prediction.sectionFeatures)
    {
        expect(
            value >= -1.0 &&
            value <= 1.0 &&
            std::isfinite(value),
            "neural prediction is normalized and finite");
    }
}

void testInferenceIsDeterministic()
{
    const auto contract =
        buildContract();

    const auto model =
        initializeCompositionNeuralModel(
            contract);

    const std::vector<double> global(
        13,
        0.15);

    const std::vector<double> section(
        6,
        0.35);

    const auto first =
        model.predictNextSection(
            global,
            section,
            true);

    const auto second =
        model.predictNextSection(
            global,
            section,
            true);

    expect(
        first.sectionFeatures ==
            second.sectionFeatures,
        "neural inference is deterministic");
}

void testInvalidInputIsRejected()
{
    const auto contract =
        buildContract();

    const auto model =
        initializeCompositionNeuralModel(
            contract);

    expect(
        !model.predictNextSection(
             std::vector<double>(
                 12,
                 0.0),
             std::vector<double>(
                 6,
                 0.0),
             true)
             .valid,
        "invalid global width is rejected");

    expect(
        !model.predictNextSection(
             std::vector<double>(
                 13,
                 0.0),
             std::vector<double>(
                 5,
                 0.0),
             true)
             .valid,
        "invalid section width is rejected");

    expect(
        !model.predictNextSection(
             std::vector<double>(
                 13,
                 0.0),
             std::vector<double>(
                 6,
                 0.0),
             false)
             .valid,
        "invalid context state is rejected");
}

void testInvalidContractPreventsInitialization()
{
    auto contract =
        buildContract();

    contract.contextLength = 0;

    const auto model =
        initializeCompositionNeuralModel(
            contract);

    expect(
        !model.initialized,
        "invalid contract prevents neural initialization");

    expect(
        !model.isValid(),
        "invalid neural model remains invalid");
}

} // namespace

int main()
{
    testDeterministicInitialization();
    testParameterShape();
    testContextResidualShape();
    testPredictionShapeAndRange();
    testInferenceIsDeterministic();
    testInvalidInputIsRejected();
    testInvalidContractPreventsInitialization();

    std::cout
        << "MIDI-GenGX Composition Neural Model tests passed.\n";

    return 0;
}
