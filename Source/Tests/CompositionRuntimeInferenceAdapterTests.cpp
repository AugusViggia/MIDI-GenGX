#include "Music/CompositionRuntimeInferenceAdapter.h"

#include <cstdlib>
#include <iostream>

using namespace midigengx::music;
using namespace midigengx::domain;

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

MusicalContext buildContext()
{
    MusicalContext context;

    context.key = Key::D;
    context.scale = Scale{
        ScaleType::Major};
    context.role = Role::Melody;

    context.parameters.lengthBars = 8;
    context.parameters.phraseLengthBars = 4;
    context.parameters.tension = 65;
    context.parameters.complexity = 55;

    context.normalize();

    return context;
}

CompositionLearningContract buildContract()
{
    // Contract dimensions only are required for this adapter. The values
    // mirror the established schema and are intentionally marked valid.
    CompositionLearningContract contract;

    contract.objective =
        LearningObjective::NextSectionPrediction;

    contract.globalInputWidth =
        CompositionDatasetSchema::globalFeatureCount;

    contract.sectionInputWidth =
        CompositionDatasetSchema::sectionFeatureCount;

    contract.targetWidth =
        CompositionDatasetSchema::sectionFeatureCount;

    contract.contextLength = 1;
    contract.usesSectionMask = true;
    contract.analysisValid = true;

    return contract;
}

void testValidFeaturesBecomeValidRequest()
{
    CompositionRuntimeFeatureAdapter featureAdapter;
    CompositionRuntimeInferenceAdapter inferenceAdapter;

    const auto features =
        featureAdapter.build(
            buildContext());

    const auto contract =
        buildContract();

    const auto request =
        inferenceAdapter.buildRequest(
            features,
            contract);

    expect(
        request.isValid(
            contract),
        "valid runtime features produce valid inference request");

    expect(
        request.contextIsValid,
        "runtime inference request marks context valid");
}

void testFeatureValuesArePreserved()
{
    CompositionRuntimeFeatureAdapter featureAdapter;
    CompositionRuntimeInferenceAdapter inferenceAdapter;

    const auto features =
        featureAdapter.build(
            buildContext());

    const auto contract =
        buildContract();

    const auto request =
        inferenceAdapter.buildRequest(
            features,
            contract);

    expect(
        request.globalFeatures ==
            features.globalFeatures,
        "global runtime features are preserved");

    expect(
        request.contextSectionFeatures ==
            features.sectionFeatures,
        "section runtime features are preserved");
}

void testInvalidFeaturesAreRejected()
{
    CompositionRuntimeInferenceAdapter inferenceAdapter;

    CompositionRuntimeFeatures invalid;
    invalid.valid = false;

    const auto request =
        inferenceAdapter.buildRequest(
            invalid,
            buildContract());

    expect(
        !request.contextIsValid,
        "invalid runtime features do not produce valid context");
}

void testContractMismatchIsRejected()
{
    CompositionRuntimeFeatureAdapter featureAdapter;
    CompositionRuntimeInferenceAdapter inferenceAdapter;

    const auto features =
        featureAdapter.build(
            buildContext());

    auto contract =
        buildContract();

    contract.globalInputWidth += 1;

    const auto request =
        inferenceAdapter.buildRequest(
            features,
            contract);

    expect(
        !request.contextIsValid,
        "contract width mismatch is rejected");
}

void testOutputRequestIsDeterministic()
{
    CompositionRuntimeFeatureAdapter featureAdapter;
    CompositionRuntimeInferenceAdapter inferenceAdapter;

    const auto features =
        featureAdapter.build(
            buildContext());

    const auto contract =
        buildContract();

    const auto first =
        inferenceAdapter.buildRequest(
            features,
            contract);

    const auto second =
        inferenceAdapter.buildRequest(
            features,
            contract);

    expect(
        first.globalFeatures ==
            second.globalFeatures &&
        first.contextSectionFeatures ==
            second.contextSectionFeatures &&
        first.contextIsValid ==
            second.contextIsValid,
        "runtime inference request construction is deterministic");
}

void testRequestDimensionsMatchContract()
{
    CompositionRuntimeFeatureAdapter featureAdapter;
    CompositionRuntimeInferenceAdapter inferenceAdapter;

    const auto features =
        featureAdapter.build(
            buildContext());

    const auto contract =
        buildContract();

    const auto request =
        inferenceAdapter.buildRequest(
            features,
            contract);

    expect(
        request.globalFeatures.size() ==
            contract.globalInputWidth,
        "global request width matches contract");

    expect(
        request.contextSectionFeatures.size() ==
            contract.sectionInputWidth,
        "section request width matches contract");
}

} // namespace

int main()
{
    testValidFeaturesBecomeValidRequest();
    testFeatureValuesArePreserved();
    testInvalidFeaturesAreRejected();
    testContractMismatchIsRejected();
    testOutputRequestIsDeterministic();
    testRequestDimensionsMatchContract();

    std::cout
        << "MIDI-GenGX Composition Runtime Inference Adapter tests passed.\n";

    return 0;
}
