#include "Music/CompositionNeuralModelArtifact.h"

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

void testValidSerialization()
{
    const auto model =
        buildModel();

    expect(
        model.isValid(),
        "source model is valid");

    const auto artifact =
        serializeCompositionNeuralModel(
            model);

    expect(
        artifact.isValid(),
        "serialized model artifact is valid");

    expect(
        !artifact.bytes.empty(),
        "serialized artifact contains bytes");
}

void testSerializedParameterCountMatchesModelShape()
{
    const auto model =
        buildModel();

    const auto artifact =
        serializeCompositionNeuralModel(
            model);

    expect(
        artifact.isValid(),
        "model artifact has valid parameter count");

    const auto expectedBytes =
        sizeof(std::uint32_t) * 8 +
        (
            model.inputWeights.size() +
            model.hiddenBias.size() +
            model.outputWeights.size() +
            model.outputBias.size() +
            model.contextResidualWeights.size()
        ) *
        sizeof(double);

    expect(
        artifact.bytes.size() ==
            expectedBytes,
        "artifact byte size matches all model parameters");
}

void testRoundTripPreservesModel()
{
    const auto original =
        buildModel();

    const auto artifact =
        serializeCompositionNeuralModel(
            original);

    CompositionNeuralModel restored;

    expect(
        deserializeCompositionNeuralModel(
            artifact,
            restored),
        "model artifact round-trip succeeds");

    expect(
        restored.isValid(),
        "restored model is valid");

    expect(
        restored.contract.globalInputWidth ==
            original.contract.globalInputWidth &&
        restored.contract.sectionInputWidth ==
            original.contract.sectionInputWidth &&
        restored.contract.targetWidth ==
            original.contract.targetWidth,
        "round-trip preserves model contract");

    expect(
        restored.inputWeights ==
            original.inputWeights &&
        restored.hiddenBias ==
            original.hiddenBias &&
        restored.outputWeights ==
            original.outputWeights &&
        restored.outputBias ==
            original.outputBias &&
        restored.contextResidualWeights ==
            original.contextResidualWeights,
        "round-trip preserves every model parameter");
}

void testRoundTripPreservesPrediction()
{
    const auto original =
        buildModel();

    const auto artifact =
        serializeCompositionNeuralModel(
            original);

    CompositionNeuralModel restored;

    expect(
        deserializeCompositionNeuralModel(
            artifact,
            restored),
        "model can be restored before prediction");

    std::vector<double> globalFeatures(
        13,
        0.25);

    std::vector<double> sectionFeatures(
        6,
        0.1);

    const auto first =
        original.predictNextSection(
            globalFeatures,
            sectionFeatures,
            true);

    const auto second =
        restored.predictNextSection(
            globalFeatures,
            sectionFeatures,
            true);

    expect(
        first.isValid(6) &&
        second.isValid(6),
        "both models produce valid predictions");

    expect(
        first.sectionFeatures ==
            second.sectionFeatures,
        "round-trip preserves exact inference output");
}

void testCorruptedHeaderIsRejected()
{
    const auto model =
        buildModel();

    auto artifact =
        serializeCompositionNeuralModel(
            model);

    artifact.bytes[0] ^=
        static_cast<std::uint8_t>(0xFF);

    expect(
        !artifact.isValid(),
        "corrupted artifact header is rejected");
}

void testTruncatedPayloadIsRejected()
{
    const auto model =
        buildModel();

    auto artifact =
        serializeCompositionNeuralModel(
            model);

    artifact.bytes.pop_back();

    expect(
        !artifact.isValid(),
        "truncated model artifact is rejected");
}

void testInvalidTargetModelIsNotSerialized()
{
    auto model =
        buildModel();

    model.outputWeights.clear();

    const auto artifact =
        serializeCompositionNeuralModel(
            model);

    expect(
        artifact.bytes.empty(),
        "invalid model is not serialized");
}

void testDeterministicSerialization()
{
    const auto firstModel =
        buildModel();

    const auto secondModel =
        buildModel();

    const auto firstArtifact =
        serializeCompositionNeuralModel(
            firstModel);

    const auto secondArtifact =
        serializeCompositionNeuralModel(
            secondModel);

    expect(
        firstArtifact.bytes ==
            secondArtifact.bytes,
        "identical models serialize deterministically");
}

} // namespace

int main()
{
    testValidSerialization();
    testSerializedParameterCountMatchesModelShape();
    testRoundTripPreservesModel();
    testRoundTripPreservesPrediction();
    testCorruptedHeaderIsRejected();
    testTruncatedPayloadIsRejected();
    testInvalidTargetModelIsNotSerialized();
    testDeterministicSerialization();

    std::cout
        << "MIDI-GenGX Composition Neural Model Artifact tests passed.\n";

    return 0;
}
