#include "Music/CompositionAIModelRuntimeProvider.h"

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

MusicalContext buildContext()
{
    MusicalContext context;

    context.key = Key::C;
    context.scale = Scale{
        ScaleType::Minor};
    context.role = Role::Lead;

    context.parameters.lengthBars = 8;
    context.parameters.phraseLengthBars = 4;
    context.parameters.tension = 65;
    context.parameters.complexity = 55;
    context.parameters.density = 55;
    context.parameters.variation = 45;

    context.normalize();

    return context;
}

void testEmptyProviderIsNotReady()
{
    CompositionAIModelRuntimeProvider provider;

    expect(
        !provider.isReady(),
        "empty AI model runtime provider is not ready");
}

void testValidArtifactMakesProviderReady()
{
    const auto model =
        buildModel();

    const auto artifact =
        serializeCompositionNeuralModel(
            model);

    CompositionAIModelRuntimeProvider provider;

    expect(
        provider.load(
            artifact),
        "valid artifact loads into AI runtime provider");

    expect(
        provider.isReady(),
        "AI runtime provider is ready after model load");
}

void testLoadedModelGeneratesValidPhrase()
{
    const auto model =
        buildModel();

    const auto artifact =
        serializeCompositionNeuralModel(
            model);

    CompositionAIModelRuntimeProvider provider;

    expect(
        provider.load(
            artifact),
        "provider loads model before generation");

    const auto phrase =
        provider.generate(
            buildContext(),
            77);

    expect(
        phrase.isValid(),
        "loaded AI model provider generates valid phrase");
}

void testGeneratedPhraseIsDeterministic()
{
    const auto model =
        buildModel();

    const auto artifact =
        serializeCompositionNeuralModel(
            model);

    CompositionAIModelRuntimeProvider provider;

    expect(
        provider.load(
            artifact),
        "provider loads deterministic model");

    const auto first =
        provider.generate(
            buildContext(),
            12345);

    const auto second =
        provider.generate(
            buildContext(),
            12345);

    expect(
        first.isValid() &&
        second.isValid(),
        "deterministic provider outputs are valid");

    expect(
        first.notes.size() ==
            second.notes.size() &&
        first.lengthBeats ==
            second.lengthBeats,
        "same seed preserves deterministic phrase shape");
}

void testDifferentSeedsRemainFunctional()
{
    const auto model =
        buildModel();

    const auto artifact =
        serializeCompositionNeuralModel(
            model);

    CompositionAIModelRuntimeProvider provider;

    expect(
        provider.load(
            artifact),
        "provider loads model for seed variation");

    const auto first =
        provider.generate(
            buildContext(),
            100);

    const auto second =
        provider.generate(
            buildContext(),
            200);

    expect(
        first.isValid() &&
        second.isValid(),
        "different seeds still generate valid phrases");
}

void testInvalidArtifactDoesNotReplaceActiveModel()
{
    const auto model =
        buildModel();

    const auto validArtifact =
        serializeCompositionNeuralModel(
            model);

    CompositionAIModelRuntimeProvider provider;

    expect(
        provider.load(
            validArtifact),
        "baseline model loads");

    auto invalidArtifact =
        validArtifact;

    invalidArtifact.bytes.pop_back();

    expect(
        !provider.load(
            invalidArtifact),
        "invalid artifact is rejected");

    expect(
        provider.isReady(),
        "active model remains ready after failed replacement");

    const auto phrase =
        provider.generate(
            buildContext(),
            9);

    expect(
        phrase.isValid(),
        "active model remains usable after failed replacement");
}

void testClearReturnsProviderToEmptyState()
{
    const auto model =
        buildModel();

    const auto artifact =
        serializeCompositionNeuralModel(
            model);

    CompositionAIModelRuntimeProvider provider;

    expect(
        provider.load(
            artifact),
        "provider loads before clear");

    provider.clear();

    expect(
        !provider.isReady(),
        "clear removes active AI model");
}

} // namespace

int main()
{
    testEmptyProviderIsNotReady();
    testValidArtifactMakesProviderReady();
    testLoadedModelGeneratesValidPhrase();
    testGeneratedPhraseIsDeterministic();
    testDifferentSeedsRemainFunctional();
    testInvalidArtifactDoesNotReplaceActiveModel();
    testClearReturnsProviderToEmptyState();

    std::cout
        << "MIDI-GenGX Composition AI Model Runtime Provider tests passed.\n";

    return 0;
}
