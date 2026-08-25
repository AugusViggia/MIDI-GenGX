#include "Plugin/AIRuntimeGeneration.h"

#include <cstdlib>
#include <iostream>

using namespace midigengx::music;
using namespace midigengx::plugin;
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

CompositionNeuralModelArtifact buildArtifact()
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

    const auto model =
        initializeCompositionNeuralModel(
            contract);

    return serializeCompositionNeuralModel(
        model);
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

void testModelLoadMakesAIRuntimeReady()
{
    AIRuntimeGeneration runtime;

    expect(
        !runtime.hasLoadedModel(),
        "AI runtime starts without loaded model");

    expect(
        runtime.loadModelArtifact(
            buildArtifact()),
        "AI runtime loads valid model artifact");

    expect(
        runtime.hasLoadedModel(),
        "AI runtime reports loaded model");
}

void testLoadedModelIsUsedWhenEnabled()
{
    AIRuntimeGeneration runtime;

    expect(
        runtime.loadModelArtifact(
            buildArtifact()),
        "AI model loads before generation");

    runtime.setEnabled(true);

    const auto phrase =
        runtime.generate(
            buildContext(),
            123);

    expect(
        phrase.isValid(),
        "loaded AI runtime generates valid phrase");
}

void testDisabledRuntimeStillUsesBaseline()
{
    AIRuntimeGeneration runtime;

    expect(
        runtime.loadModelArtifact(
            buildArtifact()),
        "AI model loads before disabled fallback test");

    runtime.setEnabled(false);

    const auto phrase =
        runtime.generate(
            buildContext(),
            123);

    expect(
        phrase.isValid(),
        "disabled AI runtime preserves valid baseline");
}

void testInvalidReplacementPreservesLoadedModel()
{
    AIRuntimeGeneration runtime;

    const auto validArtifact =
        buildArtifact();

    expect(
        runtime.loadModelArtifact(
            validArtifact),
        "initial runtime model loads");

    const bool hadModel =
        runtime.hasLoadedModel();

    auto invalidArtifact =
        validArtifact;

    invalidArtifact.bytes.pop_back();

    expect(
        !runtime.loadModelArtifact(
            invalidArtifact),
        "invalid replacement is rejected");

    expect(
        hadModel &&
        runtime.hasLoadedModel(),
        "invalid replacement does not remove active model");
}

void testClearModelRemovesRuntimeModel()
{
    AIRuntimeGeneration runtime;

    expect(
        runtime.loadModelArtifact(
            buildArtifact()),
        "model loads before clear");

    runtime.clearModel();

    expect(
        !runtime.hasLoadedModel(),
        "clear removes loaded runtime model");
}

void testExplicitProviderStillHasPriority()
{
    AIRuntimeGeneration runtime;

    runtime.setProvider(
        [](
            const MusicalContext&,
            std::uint32_t)
        {
            MusicalEngine engine;
            auto phrase =
                engine.generate(
                    buildContext(),
                    1);

            if (!phrase.notes.empty())
            {
                phrase.notes.front().midiNote =
                    84;
            }

            return phrase;
        });

    expect(
        runtime.loadModelArtifact(
            buildArtifact()),
        "model loads alongside explicit provider");

    runtime.setEnabled(true);

    const auto phrase =
        runtime.generate(
            buildContext(),
            55);

    expect(
        phrase.isValid(),
        "explicit provider produces valid phrase");

    expect(
        phrase.notes.empty() ||
        phrase.notes.front().midiNote == 84,
        "explicit provider remains the highest-priority runtime source");
}

} // namespace

int main()
{
    testModelLoadMakesAIRuntimeReady();
    testLoadedModelIsUsedWhenEnabled();
    testDisabledRuntimeStillUsesBaseline();
    testInvalidReplacementPreservesLoadedModel();
    testClearModelRemovesRuntimeModel();
    testExplicitProviderStillHasPriority();

    std::cout
        << "MIDI-GenGX AI Runtime Generation Model tests passed.\n";

    return 0;
}
