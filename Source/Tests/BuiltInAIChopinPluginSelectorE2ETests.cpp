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
    int humanization = -1;
    int noteLengthVariation = -1;
    int cadenceStrength = -1;

    int key = -1;
    int scale = -1;
    int role = -1;
    int lengthBars = -1;
    int octaveLow = -1;
    int octaveHigh = -1;
    int octaveShift = -1;

    int noteLength = -1;
    int phraseContour = -1;
    int cadenceStyle = -1;
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
            captured.humanization = context.parameters.humanization;
            captured.noteLengthVariation = context.parameters.noteLengthVariation;
            captured.cadenceStrength = context.parameters.cadenceStrength;
            captured.key = static_cast<int>(context.key);
            captured.scale = static_cast<int>(context.scale.getType());
            captured.role = static_cast<int>(context.role);
            captured.lengthBars = context.parameters.lengthBars;
            captured.octaveLow = context.parameters.octaveLow;
            captured.octaveHigh = context.parameters.octaveHigh;
            captured.octaveShift = context.parameters.octaveShift;
            captured.noteLength = static_cast<int>(context.parameters.noteLength);
            captured.phraseContour = static_cast<int>(context.parameters.phraseContour);
            captured.cadenceStyle = static_cast<int>(context.parameters.cadenceStyle);
            return provider->generate(context, seed);
        });

    processor.setKey(7);
    processor.setScale(static_cast<int>(midigengx::domain::ScaleType::Major));
    processor.setRole(static_cast<int>(midigengx::domain::Role::Piano));
    processor.setLengthBars(12);
    processor.setOctaveLow(-1);
    processor.setOctaveHigh(3);
    processor.setOctaveShift(1);

    processor.setDensity(11);
    processor.setVariation(33);
    processor.setComplexity(44);
    processor.setSyncopation(55);
    processor.setTension(66);
    processor.setRepetition(77);
    processor.setHumanization(88);
    processor.setNoteLengthVariation(19);
    processor.setCadenceStrength(91);

    processor.setAIRuntimeEnabled(true);
    processor.setGeneratorEnabled(true);
    processor.requestAIRuntimeGeneration();

    waitForGenerationToSettle(processor);

    expect(captured.received, "AI provider receives plugin generation context");

    expect(captured.density == 11, "density reaches AI provider unchanged");
    expect(captured.variation == 33, "variation reaches AI provider unchanged");
    expect(captured.complexity == 44, "complexity reaches AI provider unchanged");
    expect(captured.syncopation == 55, "syncopation reaches AI provider unchanged");
    expect(captured.tension == 66, "tension reaches AI provider unchanged");
    expect(captured.repetition == 77, "repetition reaches AI provider unchanged");
    expect(captured.humanization == 88, "humanization reaches AI provider unchanged");
    expect(captured.noteLengthVariation == 19, "note-length variation reaches AI provider unchanged");
    expect(captured.cadenceStrength == 91, "cadence strength reaches AI provider unchanged");

    expect(captured.key == 7, "key reaches AI provider unchanged");
    expect(captured.scale == static_cast<int>(midigengx::domain::ScaleType::Major),
           "scale reaches AI provider unchanged");
    expect(captured.role == static_cast<int>(midigengx::domain::Role::Piano),
           "role reaches AI provider unchanged");
    expect(captured.lengthBars == 12, "length reaches AI provider unchanged");
    expect(captured.octaveLow == -1, "octave low reaches AI provider unchanged");
    expect(captured.octaveHigh == 3, "octave high reaches AI provider unchanged");
    expect(captured.octaveShift == 1, "octave shift reaches AI provider unchanged");
    expect(captured.noteLength == static_cast<int>(midigengx::domain::NoteLength::Auto),
           "note length selector reaches AI provider unchanged");
    expect(captured.phraseContour == static_cast<int>(midigengx::domain::PhraseContour::Arch),
           "phrase contour selector reaches AI provider unchanged");
    expect(captured.cadenceStyle == static_cast<int>(midigengx::domain::CadenceStyle::Root),
           "cadence style selector reaches AI provider unchanged");
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
            captured.humanization = context.parameters.humanization;
            captured.noteLengthVariation = context.parameters.noteLengthVariation;
            captured.cadenceStrength = context.parameters.cadenceStrength;
            captured.key = static_cast<int>(context.key);
            captured.scale = static_cast<int>(context.scale.getType());
            captured.role = static_cast<int>(context.role);
            captured.lengthBars = context.parameters.lengthBars;
            captured.octaveLow = context.parameters.octaveLow;
            captured.octaveHigh = context.parameters.octaveHigh;
            captured.octaveShift = context.parameters.octaveShift;
            captured.noteLength = static_cast<int>(context.parameters.noteLength);
            captured.phraseContour = static_cast<int>(context.parameters.phraseContour);
            captured.cadenceStyle = static_cast<int>(context.parameters.cadenceStyle);
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
