#include "Plugin/PluginProcessor.h"

#include <JuceHeader.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

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

midigengx::music::CompositionNeuralModelArtifact
buildArtifact()
{
    using namespace midigengx::music;

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

void waitForGenerationToSettle(
    MIDIGenGXAudioProcessor& processor)
{
    processor.prepareToPlay(
        44100.0,
        512);

    juce::AudioBuffer<float> audioBuffer(
        2,
        512);

    juce::MidiBuffer midiBuffer;

    // `generationPending` means that a generated phrase is waiting to be
    // adopted by the audio-side phrase publication path. The worker can finish
    // before the audio callback consumes that publication, so the test must
    // exercise the same adoption boundary as the real VST3 runtime.
    for (int attempt = 0;
         attempt < 500 &&
         processor.isGenerationPending();
         ++attempt)
    {
        audioBuffer.clear();
        midiBuffer.clear();

        processor.processBlock(
            audioBuffer,
            midiBuffer);

        std::this_thread::sleep_for(
            std::chrono::milliseconds(10));
    }

    processor.releaseResources();
}

void testAIModelActivationState()
{
    MIDIGenGXAudioProcessor processor;

    const auto artifact =
        buildArtifact();

    juce::MemoryBlock bytes;
    bytes.append(
        artifact.bytes.data(),
        artifact.bytes.size());

    expect(
        processor.loadAIRuntimeModel(bytes),
        "processor loads AI model");

    expect(
        processor.hasLoadedAIRuntimeModel(),
        "processor reports loaded AI model");

    expect(
        !processor.isAIRuntimeModelActive(),
        "loaded model is not active before AI enable");

    processor.setAIRuntimeEnabled(true);

    expect(
        processor.isAIRuntimeModelActive(),
        "loaded model becomes active when AI is enabled");
}

void testAIRequestRequiresGeneratorAndAI()
{
    MIDIGenGXAudioProcessor processor;

    const auto artifact = buildArtifact();
    juce::MemoryBlock bytes;
    bytes.append(
        artifact.bytes.data(),
        artifact.bytes.size());

    expect(
        processor.loadAIRuntimeModel(bytes),
        "model loads before request guard test");

    processor.requestAIRuntimeGeneration();

    expect(
        !processor.isGenerationPending(),
        "AI request does nothing while generator is disabled");

    processor.setAIRuntimeEnabled(true);
    processor.requestAIRuntimeGeneration();

    expect(
        !processor.isGenerationPending(),
        "AI request still does nothing while generator is disabled");
}

void testEndToEndAIWorkerGenerationSettles()
{
    MIDIGenGXAudioProcessor processor;

    const auto artifact = buildArtifact();
    juce::MemoryBlock bytes;
    bytes.append(
        artifact.bytes.data(),
        artifact.bytes.size());

    expect(
        processor.loadAIRuntimeModel(bytes),
        "AI model loads before end-to-end generation");

    processor.setAIRuntimeEnabled(true);
    processor.setGeneratorEnabled(true);
    processor.requestAIRuntimeGeneration();

    expect(
        processor.isAIRuntimeModelActive(),
        "AI model is active before worker generation");

    waitForGenerationToSettle(processor);

    expect(
        !processor.isGenerationPending(),
        "AI worker generation request eventually settles");
}

void testAIModelRemainsActiveAcrossMultipleRequests()
{
    MIDIGenGXAudioProcessor processor;

    const auto artifact = buildArtifact();
    juce::MemoryBlock bytes;
    bytes.append(
        artifact.bytes.data(),
        artifact.bytes.size());

    expect(
        processor.loadAIRuntimeModel(bytes),
        "AI model loads before repeated generation");

    processor.setAIRuntimeEnabled(true);
    processor.setGeneratorEnabled(true);

    processor.requestAIRuntimeGeneration();
    waitForGenerationToSettle(processor);

    expect(
        processor.isAIRuntimeModelActive(),
        "AI model remains active after first request");

    processor.requestAIRuntimeGeneration();
    waitForGenerationToSettle(processor);

    expect(
        processor.isAIRuntimeModelActive(),
        "AI model remains active after second request");
}

void testDisablingAIReturnsToBaselineMode()
{
    MIDIGenGXAudioProcessor processor;

    const auto artifact = buildArtifact();
    juce::MemoryBlock bytes;
    bytes.append(
        artifact.bytes.data(),
        artifact.bytes.size());

    expect(
        processor.loadAIRuntimeModel(bytes),
        "AI model loads before disable test");

    processor.setAIRuntimeEnabled(true);

    expect(
        processor.isAIRuntimeModelActive(),
        "AI mode activates before disable");

    processor.setAIRuntimeEnabled(false);

    expect(
        !processor.isAIRuntimeModelActive(),
        "disabling AI returns processor to baseline mode");
}

} // namespace

int main()
{
    testAIModelActivationState();
    testAIRequestRequiresGeneratorAndAI();
    testEndToEndAIWorkerGenerationSettles();
    testAIModelRemainsActiveAcrossMultipleRequests();
    testDisablingAIReturnsToBaselineMode();

    std::cout
        << "MIDI-GenGX Plugin AI runtime end-to-end tests passed.\n";

    return 0;
}
