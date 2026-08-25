#include "Plugin/PluginProcessor.h"
#include "Music/CompositionNeuralModelArtifact.h"

#include <JuceHeader.h>

#include <cstdlib>
#include <iostream>

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

void testProcessorLoadsArtifactBytes()
{
    MIDIGenGXAudioProcessor processor;

    const auto artifact =
        buildArtifact();

    expect(
        artifact.isValid(),
        "test artifact is valid");

    juce::MemoryBlock bytes;

    bytes.append(
        artifact.bytes.data(),
        artifact.bytes.size());

    expect(
        processor.loadAIRuntimeModel(
            bytes),
        "processor loads model artifact bytes");

    expect(
        processor.hasLoadedAIRuntimeModel(),
        "processor reports loaded AI model");
}

void testProcessorRejectsInvalidBytes()
{
    MIDIGenGXAudioProcessor processor;

    juce::MemoryBlock invalid;
    const std::uint8_t data[] =
    {
        0x4D, 0x47, 0x4E, 0x58,
        0x01, 0x00, 0x00, 0x00
    };

    invalid.append(
        data,
        sizeof(data));

    expect(
        !processor.loadAIRuntimeModel(
            invalid),
        "processor rejects invalid model bytes");

    expect(
        !processor.hasLoadedAIRuntimeModel(),
        "invalid model does not become active");
}

void testProcessorLoadsArtifactFromFile()
{
    MIDIGenGXAudioProcessor processor;

    const auto artifact =
        buildArtifact();

    juce::MemoryBlock bytes;

    bytes.append(
        artifact.bytes.data(),
        artifact.bytes.size());

    const auto tempFile =
        juce::File::getSpecialLocation(
            juce::File::tempDirectory)
            .getChildFile(
                "MIDI_GenGX_Phase64_Test_Model.mgnx");

    tempFile.deleteFile();

    expect(
        tempFile.replaceWithData(
            bytes.getData(),
            bytes.getSize()),
        "temporary model artifact is written");

    expect(
        processor.loadAIRuntimeModelFromFile(
            tempFile),
        "processor loads model artifact from file");

    expect(
        processor.hasLoadedAIRuntimeModel(),
        "processor reports file-loaded AI model");

    tempFile.deleteFile();
}

void testMissingFileIsRejected()
{
    MIDIGenGXAudioProcessor processor;

    const auto missing =
        juce::File::getSpecialLocation(
            juce::File::tempDirectory)
            .getChildFile(
                "MIDI_GenGX_Phase64_Missing_Model.mgnx");

    missing.deleteFile();

    expect(
        !processor.loadAIRuntimeModelFromFile(
            missing),
        "missing model file is rejected");
}

void testClearRemovesLoadedModel()
{
    MIDIGenGXAudioProcessor processor;

    const auto artifact =
        buildArtifact();

    juce::MemoryBlock bytes;

    bytes.append(
        artifact.bytes.data(),
        artifact.bytes.size());

    expect(
        processor.loadAIRuntimeModel(
            bytes),
        "processor loads model before clear");

    processor.clearAIRuntimeModel();

    expect(
        !processor.hasLoadedAIRuntimeModel(),
        "processor clears loaded AI model");
}

void testGenerationUsesLoadedModelWhenEnabled()
{
    MIDIGenGXAudioProcessor processor;

    const auto artifact =
        buildArtifact();

    juce::MemoryBlock bytes;

    bytes.append(
        artifact.bytes.data(),
        artifact.bytes.size());

    expect(
        processor.loadAIRuntimeModel(
            bytes),
        "processor loads model before enable");

    processor.setAIRuntimeEnabled(
        true);

    expect(
        processor.isAIRuntimeEnabled(),
        "processor enables AI runtime");
}

} // namespace

int main()
{
    testProcessorLoadsArtifactBytes();
    testProcessorRejectsInvalidBytes();
    testProcessorLoadsArtifactFromFile();
    testMissingFileIsRejected();
    testClearRemovesLoadedModel();
    testGenerationUsesLoadedModelWhenEnabled();

    std::cout
        << "MIDI-GenGX Plugin AI runtime model loading tests passed.\n";

    return 0;
}
