#include "Music/CompositionNeuralModelRuntimeLoader.h"

#include <cstdlib>
#include <iostream>
#include <vector>

using namespace midigengx::music;

namespace
{

void expect(
    bool condition,
    const char* message)
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

CompositionNeuralModel buildModel()
{
    CompositionLearningContract contract;

    contract.objective =
        LearningObjective::NextSectionPrediction;
    contract.globalInputWidth = 13;
    contract.sectionInputWidth = 6;
    contract.targetWidth = 6;
    contract.contextLength = 1;
    contract.usesSectionMask = true;
    contract.analysisValid = true;

    return initializeCompositionNeuralModel(
        contract);
}

void testEmptyLoaderIsInvalid()
{
    CompositionNeuralModelRuntimeLoader loader;

    expect(
        !loader.isValid(),
        "empty runtime loader is invalid");

    expect(
        !loader.loaded,
        "empty runtime loader is not loaded");
}

void testValidArtifactLoads()
{
    const auto model =
        buildModel();

    const auto artifact =
        serializeCompositionNeuralModel(
            model);

    CompositionNeuralModelRuntimeLoader loader;

    expect(
        loader.load(
            artifact),
        "valid model artifact loads");

    expect(
        loader.isValid(),
        "runtime loader is valid after load");

    expect(
        loader.model.isValid(),
        "loaded runtime model is valid");
}

void testLoadedModelMatchesSource()
{
    const auto original =
        buildModel();

    const auto artifact =
        serializeCompositionNeuralModel(
            original);

    CompositionNeuralModelRuntimeLoader loader;

    expect(
        loader.load(
            artifact),
        "source model loads into runtime");

    expect(
        loader.model.inputWeights ==
            original.inputWeights &&
        loader.model.hiddenBias ==
            original.hiddenBias &&
        loader.model.outputWeights ==
            original.outputWeights &&
        loader.model.outputBias ==
            original.outputBias &&
        loader.model.contextResidualWeights ==
            original.contextResidualWeights,
        "runtime loader preserves all learned parameters");
}

void testLoadedModelPreservesInference()
{
    const auto original =
        buildModel();

    const auto artifact =
        serializeCompositionNeuralModel(
            original);

    CompositionNeuralModelRuntimeLoader loader;

    expect(
        loader.load(
            artifact),
        "model loads before inference");

    const std::vector<double> globalFeatures(
        13,
        0.35);

    const std::vector<double> sectionFeatures(
        6,
        -0.15);

    const auto originalPrediction =
        original.predictNextSection(
            globalFeatures,
            sectionFeatures,
            true);

    const auto runtimePrediction =
        loader.model.predictNextSection(
            globalFeatures,
            sectionFeatures,
            true);

    expect(
        originalPrediction.isValid(6) &&
        runtimePrediction.isValid(6),
        "source and runtime predictions are valid");

    expect(
        originalPrediction.sectionFeatures ==
            runtimePrediction.sectionFeatures,
        "runtime loader preserves exact neural inference");
}

void testInvalidArtifactDoesNotMutateLoadedModel()
{
    const auto original =
        buildModel();

    const auto validArtifact =
        serializeCompositionNeuralModel(
            original);

    CompositionNeuralModelRuntimeLoader loader;

    expect(
        loader.load(
            validArtifact),
        "baseline runtime model loads");

    auto invalidArtifact =
        validArtifact;

    invalidArtifact.bytes.pop_back();

    const auto before =
        loader.model.outputWeights;

    expect(
        !loader.load(
            invalidArtifact),
        "invalid artifact is rejected");

    expect(
        loader.isValid(),
        "previously loaded model remains valid after failed load");

    expect(
        loader.model.outputWeights ==
            before,
        "failed load does not mutate active runtime model");
}

void testClearUnloadsModel()
{
    const auto model =
        buildModel();

    const auto artifact =
        serializeCompositionNeuralModel(
            model);

    CompositionNeuralModelRuntimeLoader loader;

    expect(
        loader.load(
            artifact),
        "model loads before clear");

    loader.clear();

    expect(
        !loader.loaded &&
        !loader.isValid(),
        "clear removes active runtime model");
}

void testRepeatedLoadReplacesModelAtomically()
{
    auto firstModel =
        buildModel();

    auto secondModel =
        buildModel();

    secondModel.outputBias[0] =
        0.25;

    const auto firstArtifact =
        serializeCompositionNeuralModel(
            firstModel);

    const auto secondArtifact =
        serializeCompositionNeuralModel(
            secondModel);

    CompositionNeuralModelRuntimeLoader loader;

    expect(
        loader.load(
            firstArtifact),
        "first runtime model loads");

    expect(
        loader.load(
            secondArtifact),
        "second runtime model replaces first");

    expect(
        loader.model.outputBias ==
            secondModel.outputBias,
        "second runtime model becomes active");
}

} // namespace

int main()
{
    testEmptyLoaderIsInvalid();
    testValidArtifactLoads();
    testLoadedModelMatchesSource();
    testLoadedModelPreservesInference();
    testInvalidArtifactDoesNotMutateLoadedModel();
    testClearUnloadsModel();
    testRepeatedLoadReplacesModelAtomically();

    std::cout
        << "MIDI-GenGX Composition Neural Model Runtime Loader tests passed.\n";

    return 0;
}
