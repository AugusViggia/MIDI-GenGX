#pragma once

#include <JuceHeader.h>

#include <array>
#include <atomic>
#include <cstdint>
#include <memory>

#include "../Domain/MusicalContext.h"
#include "../Music/Phrase.h"
#include "../Generation/GenerationActivationPolicy.h"
#include "AIRuntimeGeneration.h"

namespace midigengx::generation { class PhraseGenerationWorker; }

class MIDIGenGXAudioProcessor : public juce::AudioProcessor
{
public:
    MIDIGenGXAudioProcessor();

    ~MIDIGenGXAudioProcessor() override;

    void prepareToPlay(double sampleRate,
                       int samplesPerBlock) override;

    void releaseResources() override;

    bool isBusesLayoutSupported(
        const BusesLayout& layouts) const override;

    void processBlock(
        juce::AudioBuffer<float>&,
        juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;

    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(
        int index,
        const juce::String& newName) override;

    void getStateInformation(
        juce::MemoryBlock& destData) override;

    void setStateInformation(
        const void* data,
        int sizeInBytes) override;

    // Generator/transport state.
    void setGeneratorEnabled(bool enabled);
    bool isGeneratorEnabled() const noexcept;
    bool isTransportPlaying() const noexcept;

    // Musical context controls. UI writes only simple atomics;
    // the audio thread snapshots them at safe points.
    void setKey(int pitchClass) noexcept;
    int getKey() const noexcept;

    void setScale(int scaleType) noexcept;
    int getScale() const noexcept;

    void setRole(int role) noexcept;
    int getRole() const noexcept;

    void setLengthBars(int bars) noexcept;
    int getLengthBars() const noexcept;

    void setDensity(int value) noexcept;
    int getDensity() const noexcept;

    void setVariation(int value) noexcept;
    int getVariation() const noexcept;

    void setComplexity(int value) noexcept;
    int getComplexity() const noexcept;

    void setSyncopation(int value) noexcept;
    int getSyncopation() const noexcept;

    void setTension(int value) noexcept;
    int getTension() const noexcept;

    void setRepetition(int value) noexcept;
    int getRepetition() const noexcept;

    void setHumanization(int value) noexcept;
    int getHumanization() const noexcept;

    void setNoteLengthVariation(int value) noexcept;
    int getNoteLengthVariation() const noexcept;

    void setNoteLength(int length) noexcept;
    int getNoteLength() const noexcept;

    void setPhraseContour(int contour) noexcept;
    int getPhraseContour() const noexcept;

    void setCadenceStyle(int cadence) noexcept;
    int getCadenceStyle() const noexcept;

    void setCadenceStrength(int value) noexcept;
    int getCadenceStrength() const noexcept;

    void setGenrePreset(int genre) noexcept;
    int getGenrePreset() const noexcept;

    void applyGenrePreset(int genre) noexcept;

    void setOctaveLow(int octave) noexcept;
    int getOctaveLow() const noexcept;

    void setOctaveHigh(int octave) noexcept;
    int getOctaveHigh() const noexcept;

    void setOctaveShift(int shift) noexcept;
    int getOctaveShift() const noexcept;

    // Queues a phrase regeneration. If transport is running, the current
    // phrase continues uninterrupted and the queued phrase is applied when
    // playback stops.
    void requestGeneration() noexcept;

    bool isGenerationPending() const noexcept;

    void requestAIRuntimeGeneration() noexcept;

    bool isAIRuntimeModelActive() const noexcept;

    void setAIRuntimeEnabled(
        bool enabled) noexcept;

    bool isAIRuntimeEnabled() const noexcept;

    void setAIRuntimeGenerationProvider(
        midigengx::plugin::AIRuntimeGeneration::GenerationProvider provider);

    bool loadAIRuntimeModel(
        const juce::MemoryBlock& artifactBytes);

    bool loadAIRuntimeModelFromFile(
        const juce::File& artifactFile);

    bool hasLoadedAIRuntimeModel() const noexcept;

    void clearAIRuntimeModel();

private:
    double sampleRate = 44100.0;
    double lastPpqPosition = 0.0;
    int activeNoteMidi = -1;
    double activeNoteOffPpq = -1.0;

    std::atomic<bool> generatorEnabled{false};
    std::atomic<bool> stopGenerationRequested{false};
    bool previousPlayingState = false;
    bool previousRecordingState = false;
    std::atomic<bool> transportPlayingState{false};

    std::array<midigengx::music::Phrase, 2> phraseBuffers;
    std::atomic<int> publishedPhraseBuffer{-1};
    std::atomic<std::uint64_t> publishedPhraseRequestId{0};
    std::atomic<int> audioReadPhraseBuffer{-1};
    std::uint64_t activePhraseRequestId = 0;
    int activePhraseBuffer = -1;
    bool phraseReady = false;

    std::atomic<int> keyPitchClass{0};
    std::atomic<int> scaleType{
        static_cast<int>(midigengx::domain::ScaleType::Minor)
    };
    std::atomic<int> roleType{
        static_cast<int>(midigengx::domain::Role::Lead)
    };
    std::atomic<int> lengthBars{4};
    std::atomic<int> density{50};
    std::atomic<int> variation{25};
    std::atomic<int> complexity{50};
    std::atomic<int> syncopation{50};
    std::atomic<int> tension{50};
    std::atomic<int> repetition{50};
    std::atomic<int> humanization{0};
    std::atomic<int> noteLengthVariation{0};
    std::atomic<int> noteLength{
        static_cast<int>(
            midigengx::domain::NoteLength::Auto)
    };
    std::atomic<int> phraseContour{
        static_cast<int>(
            midigengx::domain::PhraseContour::Arch)
    };
    std::atomic<int> cadenceStyle{
        static_cast<int>(
            midigengx::domain::CadenceStyle::Root)
    };
    std::atomic<int> cadenceStrength{75};
    std::atomic<int> genrePreset{
        static_cast<int>(
            midigengx::domain::GenrePreset::Custom)
    };
    std::atomic<int> octaveLow{2};
    std::atomic<int> octaveHigh{4};
    std::atomic<int> octaveShift{0};

    std::atomic<bool> generationPending{false};
    std::atomic<std::uint64_t> generationRequestId{0};
    std::atomic<std::uint32_t> seedCounter{0x13579BDFu};
    bool restoringState = false;

    std::unique_ptr<midigengx::generation::PhraseGenerationWorker> generationWorker;
    midigengx::plugin::AIRuntimeGeneration aiRuntimeGeneration;

    midigengx::domain::MusicalContext buildMusicalContextSnapshot() const;
    void requestPhraseGeneration() noexcept;
    void publishGeneratedPhrase(midigengx::music::Phrase&&,std::uint64_t);
    void adoptPublishedPhraseIfSafe(
        double currentPpq,
        bool transportPlaying);
    void resetGeneration();
    void markContextChanged() noexcept;

    void emitPhraseMidi(
        juce::MidiBuffer& midiMessages,
        int numSamples);

    void stopActiveNote(
        juce::MidiBuffer& midiMessages);

    juce::Optional<juce::AudioPlayHead::PositionInfo>
    getPositionInfo() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        MIDIGenGXAudioProcessor
    )
};
