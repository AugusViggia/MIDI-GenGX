#include "Plugin/PluginProcessor.h"
#include "Music/BuiltInAIModel.h"
#include "Music/CompositionConditionedSequenceNeuralModelRuntimeProvider.h"
#include "Music/CompositionConditionedSequenceNeuralModelArtifact.h"

#include <JuceHeader.h>

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>

namespace
{
void expect(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}

juce::MemoryBlock builtInModelBytes()
{
    return juce::MemoryBlock(
        midigengx::music::built_in_ai_model::data(),
        midigengx::music::built_in_ai_model::size());
}

void waitForGenerationToSettle(MIDIGenGXAudioProcessor& processor)
{
    processor.prepareToPlay(44100.0, 512);

    juce::AudioBuffer<float> audioBuffer(2, 512);
    juce::MidiBuffer midiBuffer;

    for (int attempt = 0;
         attempt < 500 && processor.isGenerationPending();
         ++attempt)
    {
        audioBuffer.clear();
        midiBuffer.clear();
        processor.processBlock(audioBuffer, midiBuffer);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    expect(
        !processor.isGenerationPending(),
        "plugin generation request eventually settles");

    processor.releaseResources();
}

struct CapturedContext
{
    bool received = false;
    int density = -1;
    int variation = -1;
    int complexity = -1;
    int syncopation = -1;
    int tension = -1;
    int repetition = -1;
};

void testEmbeddedChopinModelIsLoadedAtConstruction()
{
    MIDIGenGXAudioProcessor processor;

    expect(
        processor.hasLoadedAIRuntimeModel(),
        "embedded Chopin model loads during processor construction");

    expect(
        processor.isAIRuntimeModelActive(),
        "embedded Chopin model is active during processor construction");
}

void testRealPluginSelectorsReachAIProvider()
{
    MIDIGenGXAudioProcessor processor;

    const auto bytes = builtInModelBytes();
    expect(bytes.getSize() > 0, "embedded Chopin model bytes exist");

    midigengx::music::CompositionConditionedSequenceNeuralModelArtifact artifact;
    artifact.bytes.resize(bytes.getSize());
    std::memcpy(
        artifact.bytes.data(),
        bytes.getData(),
        bytes.getSize());

    auto provider = std::make_shared<
        midigengx::music::CompositionConditionedSequenceNeuralModelRuntimeProvider>();

    expect(
        provider->load(artifact),
        "embedded Chopin model loads into conditioned runtime provider");

    CapturedContext captured;

    processor.setAIRuntimeGenerationProvider(
        [&provider, &captured](
            const midigengx::domain::MusicalContext& context,
            std::uint32_t seed)
            -> midigengx::music::Phrase
        {
            captured.received = true;
            captured.density = context.parameters.density;
            captured.variation = context.parameters.variation;
            captured.complexity = context.parameters.complexity;
            captured.syncopation = context.parameters.syncopation;
            captured.tension = context.parameters.tension;
            captured.repetition = context.parameters.repetition;
            return provider->generate(context, seed);
        });

    processor.setDensity(11);
    processor.setVariation(22);
    processor.setComplexity(33);
    processor.setSyncopation(44);
    processor.setTension(55);
    processor.setRepetition(66);
    processor.setAIRuntimeEnabled(true);
    processor.setGeneratorEnabled(true);
    processor.requestAIRuntimeGeneration();

    waitForGenerationToSettle(processor);

    expect(captured.received, "AI provider receives plugin generation context");
    expect(captured.density == 11, "density reaches AI provider unchanged");
    expect(captured.variation == 22, "variation reaches AI provider unchanged");
    expect(captured.complexity == 33, "complexity reaches AI provider unchanged");
    expect(captured.syncopation == 44, "syncopation reaches AI provider unchanged");
    expect(captured.tension == 55, "tension reaches AI provider unchanged");
    expect(captured.repetition == 66, "repetition reaches AI provider unchanged");
}

void testSelectorChangesTriggerNewAIRequest()
{
    MIDIGenGXAudioProcessor processor;
    CapturedContext captured;

    const auto bytes = builtInModelBytes();
    midigengx::music::CompositionConditionedSequenceNeuralModelArtifact artifact;
    artifact.bytes.resize(bytes.getSize());
    std::memcpy(
        artifact.bytes.data(),
        bytes.getData(),
        bytes.getSize());

    auto provider = std::make_shared<
        midigengx::music::CompositionConditionedSequenceNeuralModelRuntimeProvider>();
    expect(provider->load(artifact), "provider loads for selector rerun test");

    processor.setAIRuntimeGenerationProvider(
        [&provider, &captured](
            const midigengx::domain::MusicalContext& context,
            std::uint32_t seed)
            -> midigengx::music::Phrase
        {
            captured.received = true;
            captured.density = context.parameters.density;
            captured.variation = context.parameters.variation;
            captured.complexity = context.parameters.complexity;
            captured.syncopation = context.parameters.syncopation;
            captured.tension = context.parameters.tension;
            captured.repetition = context.parameters.repetition;
            return provider->generate(context, seed);
        });

    processor.setAIRuntimeEnabled(true);
    processor.setGeneratorEnabled(true);

    processor.setDensity(20);
    processor.requestAIRuntimeGeneration();
    waitForGenerationToSettle(processor);
    expect(captured.received, "first selector profile reaches AI provider");
    expect(captured.density == 20, "first selector profile is observed");

    processor.setDensity(80);
    processor.requestAIRuntimeGeneration();
    waitForGenerationToSettle(processor);
    expect(captured.density == 80, "changed selector profile reaches next AI request");
}

} // namespace

int main()
{
    testEmbeddedChopinModelIsLoadedAtConstruction();
    testRealPluginSelectorsReachAIProvider();
    testSelectorChangesTriggerNewAIRequest();

    std::cout
        << "MIDI-GenGX built-in Chopin AI plugin selector E2E tests passed.\n";
    return 0;
}
