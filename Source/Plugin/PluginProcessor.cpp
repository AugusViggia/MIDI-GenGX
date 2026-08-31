#include <cstring>
#include "PluginProcessor.h"
#include "../Music/BuiltInAIModel.h"
#include "../Generation/GenerationActivationPolicy.h"
#include "../Domain/GenrePresets.h"
#include "../Generation/PhraseGenerationWorker.h"
#include "PluginEditor.h"

namespace
{
constexpr auto generatorEnabledKey = "generatorEnabled";
constexpr auto keyKey = "keyPitchClass";
constexpr auto scaleKey = "scaleType";
constexpr auto roleKey = "roleType";
constexpr auto lengthKey = "lengthBars";
constexpr auto densityKey = "density";
constexpr auto variationKey = "variation";
constexpr auto complexityKey = "complexity";
constexpr auto syncopationKey = "syncopation";
constexpr auto tensionKey = "tension";
constexpr auto repetitionKey = "repetition";
constexpr auto humanizationKey = "humanization";
constexpr auto noteLengthVariationKey = "noteLengthVariation";
constexpr auto noteLengthKey = "noteLength";
constexpr auto phraseContourKey = "phraseContour";
constexpr auto cadenceStyleKey = "cadenceStyle";
constexpr auto cadenceStrengthKey = "cadenceStrength";
constexpr auto genrePresetKey = "genrePreset";
constexpr auto octaveLowKey = "octaveLow";
constexpr auto octaveHighKey = "octaveHigh";
constexpr auto octaveShiftKey = "octaveShift";
}

MIDIGenGXAudioProcessor::MIDIGenGXAudioProcessor()
    : AudioProcessor(
        BusesProperties()
            .withOutput(
                "Audio Output",
                juce::AudioChannelSet::stereo(),
                true))
{
    // Initial V1 context.
    keyPitchClass.store(0);
    scaleType.store(
        static_cast<int>(
            midigengx::domain::ScaleType::Minor));
    roleType.store(
        static_cast<int>(
            midigengx::domain::Role::Lead));
    lengthBars.store(4);
    density.store(50);
    variation.store(25);
    complexity.store(50);
    syncopation.store(50);
    tension.store(50);
    repetition.store(50);
    humanization.store(0);
    noteLengthVariation.store(0);
    noteLength.store(
        static_cast<int>(
            midigengx::domain::NoteLength::Auto));
    phraseContour.store(
        static_cast<int>(
            midigengx::domain::PhraseContour::Arch));
    cadenceStyle.store(
        static_cast<int>(
            midigengx::domain::CadenceStyle::Root));
    cadenceStrength.store(75);
    octaveLow.store(2);
    octaveHigh.store(4);
    octaveShift.store(0);

    const juce::MemoryBlock builtInModel(
        midigengx::music::built_in_ai_model::data(),
        midigengx::music::built_in_ai_model::size());

    if (loadAIRuntimeModel(builtInModel))
        aiRuntimeGeneration.setEnabled(true);

    generationWorker =
        std::make_unique<midigengx::generation::PhraseGenerationWorker>(
            [this]()
            {
                return buildMusicalContextSnapshot();
            },
            [this](
                midigengx::music::Phrase&& phrase,
                std::uint64_t id)
            {
                publishGeneratedPhrase(
                    std::move(phrase),
                    id);
            },
            [this](
                const midigengx::domain::MusicalContext& context,
                std::uint32_t seed)
            {
                return aiRuntimeGeneration.generate(
                    context,
                    seed);
            });
}

MIDIGenGXAudioProcessor::~MIDIGenGXAudioProcessor() = default;

void MIDIGenGXAudioProcessor::prepareToPlay(
    double newSampleRate,
    int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);

    sampleRate =
        newSampleRate > 0.0
            ? newSampleRate
            : 44100.0;

    lastPpqPosition = 0.0;
    previousPlayingState = false;
    previousRecordingState = false;
    transportPlayingState.store(false, std::memory_order_release);

    generationPending.store(false);
    generationRequestId.fetch_add(1, std::memory_order_acq_rel);
    stopGenerationRequested.store(false);

    if (generationWorker)
        generationWorker->invalidate();

    resetGeneration();

    if (generatorEnabled.load())
        requestPhraseGeneration();
}

void MIDIGenGXAudioProcessor::releaseResources()
{
    generationRequestId.fetch_add(1, std::memory_order_acq_rel);

    if (generationWorker)
        generationWorker->invalidate();

    generationPending.store(false, std::memory_order_release);
    resetGeneration();
}

bool MIDIGenGXAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const
{
    const auto output =
        layouts.getMainOutputChannelSet();

    const auto input =
        layouts.getMainInputChannelSet();

    return output ==
               juce::AudioChannelSet::stereo() &&
           input.isDisabled();
}

void MIDIGenGXAudioProcessor::processBlock(
    juce::AudioBuffer<float>& audioBuffer,
    juce::MidiBuffer& midiMessages)
{
    audioBuffer.clear();

    const auto position = getPositionInfo();
    const bool isPlaying =
        position.hasValue() && position->getIsPlaying();
    const bool isRecording =
        position.hasValue() && position->getIsRecording();

    transportPlayingState.store(
        isPlaying,
        std::memory_order_release);

    const bool transportStopDetected =
        (previousPlayingState && !isPlaying) ||
        (previousRecordingState && !isRecording);

    if (stopGenerationRequested.exchange(false))
    {
        stopActiveNote(midiMessages);
        resetGeneration();

        if (!generatorEnabled.load(std::memory_order_acquire))
            generationPending.store(false, std::memory_order_release);

        previousPlayingState = isPlaying;
        previousRecordingState = isRecording;
        return;
    }

    if (transportStopDetected)
    {
        stopActiveNote(midiMessages);
        generatorEnabled.store(
            false,
            std::memory_order_release);
        generationPending.store(
            false,
            std::memory_order_release);
        generationRequestId.fetch_add(
            1,
            std::memory_order_acq_rel);

        if (generationWorker)
            generationWorker->invalidate();

        resetGeneration();
        previousPlayingState = isPlaying;
        previousRecordingState = isRecording;
        return;
    }

    if (!generatorEnabled.load())
    {
        previousPlayingState = isPlaying;
        previousRecordingState = isRecording;
        return;
    }

    if (previousPlayingState && !isPlaying)
    {
        stopActiveNote(midiMessages);
        lastPpqPosition = 0.0;
        adoptPublishedPhraseIfSafe(
            position.hasValue() &&
                    position->getPpqPosition().hasValue()
                ? *position->getPpqPosition()
                : 0.0,
            false);
    }

    previousPlayingState = isPlaying;
    previousRecordingState = isRecording;

    const double currentPpq =
        position.hasValue() &&
                position->getPpqPosition().hasValue()
            ? *position->getPpqPosition()
            : 0.0;

    // A newly generated phrase may be adopted immediately when there is no
    // active phrase. When replacing an active phrase during playback, the
    // swap is deferred until the next phrase boundary.
    adoptPublishedPhraseIfSafe(
        currentPpq,
        isPlaying);

    if (!isPlaying)
        return;

    if (phraseReady)
        emitPhraseMidi(
            midiMessages,
            audioBuffer.getNumSamples());
}

midigengx::domain::MusicalContext
MIDIGenGXAudioProcessor::buildMusicalContextSnapshot() const
{
    midigengx::domain::MusicalContext context;
    context.key = midigengx::domain::keyFromPitchClass(keyPitchClass.load());
    context.scale = midigengx::domain::Scale(static_cast<midigengx::domain::ScaleType>(scaleType.load()));
    context.role = static_cast<midigengx::domain::Role>(roleType.load());
    auto& p = context.parameters;
    p.lengthBars = juce::jlimit(1,64,lengthBars.load());
    p.phraseLengthBars = std::min(4,p.lengthBars);
    p.density = juce::jlimit(0,100,density.load());
    p.variation = juce::jlimit(0,100,variation.load());
    p.complexity = juce::jlimit(0,100,complexity.load());
    p.syncopation = juce::jlimit(0,100,syncopation.load());
    p.tension = juce::jlimit(0,100,tension.load());
    p.repetition = juce::jlimit(0,100,repetition.load());
    p.humanization = juce::jlimit(0,100,humanization.load());
    p.noteLengthVariation = juce::jlimit(0,100,noteLengthVariation.load());
    p.noteLength = static_cast<midigengx::domain::NoteLength>(juce::jlimit(0,static_cast<int>(midigengx::domain::NoteLength::Custom),noteLength.load()));
    p.phraseContour = static_cast<midigengx::domain::PhraseContour>(juce::jlimit(0,static_cast<int>(midigengx::domain::PhraseContour::Custom),phraseContour.load()));
    p.cadenceStyle = static_cast<midigengx::domain::CadenceStyle>(juce::jlimit(0,static_cast<int>(midigengx::domain::CadenceStyle::Custom),cadenceStyle.load()));
    p.cadenceStrength = juce::jlimit(0,100,cadenceStrength.load());
    p.genrePreset = static_cast<midigengx::domain::GenrePreset>(juce::jlimit(0,static_cast<int>(midigengx::domain::GenrePreset::Count)-1,genrePreset.load()));
    p.octaveLow = juce::jlimit(-8,8,octaveLow.load());
    p.octaveHigh = juce::jlimit(-8,8,octaveHigh.load());
    p.octaveShift = juce::jlimit(-2,2,octaveShift.load());
    context.normalize();
    return context;
}

void MIDIGenGXAudioProcessor::requestPhraseGeneration() noexcept
{
    generationPending.store(true,std::memory_order_release);
    const auto requestId = generationRequestId.fetch_add(1,std::memory_order_acq_rel)+1;
    const auto seed = seedCounter.fetch_add(0x9E3779B9u,std::memory_order_relaxed) ^ static_cast<std::uint32_t>(requestId);
    if (generationWorker) generationWorker->request(requestId,seed);
}

void MIDIGenGXAudioProcessor::publishGeneratedPhrase(midigengx::music::Phrase&& phrase,std::uint64_t requestId)
{
    if (!generatorEnabled.load(std::memory_order_acquire)) return;
    if (requestId != generationRequestId.load(std::memory_order_acquire)) return;
    const int published=publishedPhraseBuffer.load(std::memory_order_acquire);
    int reading=audioReadPhraseBuffer.load(std::memory_order_acquire);
    const int target=published==0 ? 1 : 0;
    while (target==reading)
    {
        if (generationWorker==nullptr) return;
        juce::Thread::sleep(1);
        reading=audioReadPhraseBuffer.load(std::memory_order_acquire);
    }
    phraseBuffers[static_cast<std::size_t>(target)] = std::move(phrase);
    publishedPhraseRequestId.store(requestId,std::memory_order_release);
    publishedPhraseBuffer.store(target,std::memory_order_release);
    generationPending.store(true,std::memory_order_release);
}

void MIDIGenGXAudioProcessor::adoptPublishedPhraseIfSafe(
    double currentPpq,
    bool transportPlaying)
{
    const int published =
        publishedPhraseBuffer.load(
            std::memory_order_acquire);

    if (published < 0)
        return;

    const auto publishedId =
        publishedPhraseRequestId.load(
            std::memory_order_acquire);

    const auto currentId =
        generationRequestId.load(
            std::memory_order_acquire);

    if (publishedId != currentId)
        return;

    if (published == activePhraseBuffer &&
        phraseReady)
    {
        generationPending.store(
            false,
            std::memory_order_release);
        return;
    }

    const auto& candidate =
        phraseBuffers[
            static_cast<std::size_t>(
                published)];

    if (!midigengx::generation::
            shouldAdoptPublishedPhrase(
                phraseReady,
                transportPlaying,
                currentPpq,
                lastPpqPosition,
                candidate.lengthBeats))
    {
        return;
    }

    activePhraseBuffer = published;
    activePhraseRequestId = publishedId;
    phraseReady = true;
    generationPending.store(
        false,
        std::memory_order_release);

    // When a new phrase becomes active during playback, the phrase engine
    // will naturally start at the next musical cycle because emitPhraseMidi
    // maps notes to absolute PPQ cycles. Do not reset lastPpqPosition here.
    if (!transportPlaying)
        lastPpqPosition = 0.0;
}

void MIDIGenGXAudioProcessor::resetGeneration()
{
    phraseReady=false;
    activePhraseBuffer=-1;
    activePhraseRequestId=0;
    publishedPhraseBuffer.store(-1,std::memory_order_release);
    publishedPhraseRequestId.store(0,std::memory_order_release);
    lastPpqPosition=0.0;
    activeNoteMidi=-1;
    activeNoteOffPpq=-1.0;
}

void MIDIGenGXAudioProcessor::markContextChanged() noexcept
{
    generationPending.store(true,std::memory_order_release);
    if (!restoringState &&
        generatorEnabled.load(std::memory_order_acquire) &&
        transportPlayingState.load(std::memory_order_acquire))
    {
        requestPhraseGeneration();
    }
}

void MIDIGenGXAudioProcessor::stopActiveNote(
    juce::MidiBuffer& midiMessages)
{
    // Always terminate the last generated note on the audio thread.
    // CC123/CC120 are sent as a defensive MIDI reset for hosts/instruments
    // that may continue a voice when a Note Off lands exactly at a block edge.
    constexpr int midiChannel = 1;

    if (activeNoteMidi >= 0)
    {
            midiMessages.addEvent(
                juce::MidiMessage::noteOff(
                    midiChannel,
                    activeNoteMidi),
                0);

        activeNoteMidi = -1;
        activeNoteOffPpq = -1.0;
    }

    // Defensive host reset: stop any potentially sustained controller-driven
    // voice on the generator channel as well.
    midiMessages.addEvent(
        juce::MidiMessage::controllerEvent(
            midiChannel,
            123,
            0),
        0);
}


juce::Optional<juce::AudioPlayHead::PositionInfo>
MIDIGenGXAudioProcessor::getPositionInfo() const
{
    if (auto* hostPlayHead = getPlayHead())
        return hostPlayHead->getPosition();

    return {};
}

void MIDIGenGXAudioProcessor::emitPhraseMidi(
    juce::MidiBuffer& midiMessages,
    int numSamples)
{
    const int phraseIndex = activePhraseBuffer;
    if (phraseIndex < 0 || phraseIndex >= static_cast<int>(phraseBuffers.size()) || numSamples <= 0 ||
        sampleRate <= 0.0)
    {
        return;
    }

    audioReadPhraseBuffer.store(phraseIndex,std::memory_order_release);
    struct AudioPhraseReadGuard
    {
        std::atomic<int>& flag;
        ~AudioPhraseReadGuard(){ flag.store(-1,std::memory_order_release); }
    } guard{audioReadPhraseBuffer};
    const auto& generatedPhrase = phraseBuffers[static_cast<std::size_t>(phraseIndex)];
    if (generatedPhrase.notes.empty()) return;

    const auto position =
        getPositionInfo();

    if (!position.hasValue())
        return;

    const auto ppq =
        position->getPpqPosition();

    const auto bpm =
        position->getBpm();

    if (!ppq.hasValue() ||
        !bpm.hasValue() ||
        *bpm <= 0.0)
        return;

    const double currentPpq = *ppq;

    const double beatsPerSecond =
        *bpm / 60.0;

    const double blockBeats =
        (static_cast<double>(numSamples) /
         sampleRate) *
        beatsPerSecond;

    const double blockEnd =
        currentPpq + blockBeats;

    if (currentPpq + 1.0e-6 < lastPpqPosition)
        stopActiveNote(midiMessages);

    if (activeNoteMidi >= 0 &&
        activeNoteOffPpq >= currentPpq - 1.0e-9 &&
        activeNoteOffPpq < blockEnd)
    {
        const double offsetBeats =
            std::max(
                0.0,
                activeNoteOffPpq - currentPpq);

        const int offset =
            juce::jlimit(
                0,
                numSamples - 1,
                static_cast<int>(
                    std::llround(
                        (offsetBeats /
                         beatsPerSecond) *
                        sampleRate)));

        midiMessages.addEvent(
            juce::MidiMessage::noteOff(
                1,
                activeNoteMidi),
            offset);

        activeNoteMidi = -1;
        activeNoteOffPpq = -1.0;
    }

    const double phraseLength =
        generatedPhrase.lengthBeats;

    if (phraseLength <= 0.0)
        return;

    const long long firstCycle =
        static_cast<long long>(
            std::floor(
                currentPpq /
                phraseLength));

    const long long lastCycle =
        static_cast<long long>(
            std::floor(
                (std::max(
                    currentPpq,
                    blockEnd - 1.0e-12)) /
                phraseLength));

    for (long long cycle = firstCycle;
         cycle <= lastCycle;
         ++cycle)
    {
        const double cycleOffset =
            static_cast<double>(cycle) *
            phraseLength;

        for (const auto& note :
             generatedPhrase.notes)
        {
            const double notePpq =
                cycleOffset +
                note.startBeat;

            if (notePpq < currentPpq - 1.0e-9 ||
                notePpq >= blockEnd - 1.0e-12)
            {
                continue;
            }

            const double noteOffsetBeats =
                std::max(
                    0.0,
                    notePpq - currentPpq);

            const int noteOnOffset =
                juce::jlimit(
                    0,
                    numSamples - 1,
                    static_cast<int>(
                        std::llround(
                            (noteOffsetBeats /
                             beatsPerSecond) *
                            sampleRate)));

            if (activeNoteMidi >= 0)
            {
                midiMessages.addEvent(
                    juce::MidiMessage::noteOff(
                        1,
                        activeNoteMidi),
                    noteOnOffset);

                activeNoteMidi = -1;
                activeNoteOffPpq = -1.0;
                    }



            midiMessages.addEvent(
                juce::MidiMessage::noteOn(
                    note.channel,
                    note.midiNote,
                    static_cast<juce::uint8>(
                        note.velocity)),
                noteOnOffset);

            activeNoteMidi =
                note.midiNote;

            activeNoteOffPpq =
                notePpq +
                note.durationBeats;

            if (activeNoteOffPpq <
                blockEnd)
            {
                const double offBeats =
                    std::max(
                        0.0,
                        activeNoteOffPpq -
                        currentPpq);

                const int noteOffOffset =
                    juce::jlimit(
                        0,
                        numSamples - 1,
                        static_cast<int>(
                            std::llround(
                                (offBeats /
                                 beatsPerSecond) *
                                sampleRate)));

                midiMessages.addEvent(
                    juce::MidiMessage::noteOff(
                        note.channel,
                        note.midiNote),
                    noteOffOffset);

                activeNoteMidi = -1;
                activeNoteOffPpq = -1.0;
            }
        }
    }

    lastPpqPosition = blockEnd;
}

void MIDIGenGXAudioProcessor::setGeneratorEnabled(
    bool enabled)
{
    const bool wasEnabled = generatorEnabled.exchange(enabled,std::memory_order_acq_rel);
    if (enabled && !wasEnabled)
    {
        requestPhraseGeneration();
        return;
    }
    if (!enabled && wasEnabled)
    {
        generationPending.store(false,std::memory_order_release);
        generationRequestId.fetch_add(1,std::memory_order_acq_rel);
        stopGenerationRequested.store(true,std::memory_order_release);
        if (generationWorker) generationWorker->invalidate();
    }
}

bool MIDIGenGXAudioProcessor::isGeneratorEnabled() const noexcept
{
    return generatorEnabled.load();
}

void MIDIGenGXAudioProcessor::requestAIRuntimeGeneration() noexcept
{
    if (!generatorEnabled.load(
            std::memory_order_acquire) ||
        !aiRuntimeGeneration.isEnabled())
    {
        return;
    }

    requestPhraseGeneration();
}

bool MIDIGenGXAudioProcessor::isAIRuntimeModelActive()
    const noexcept
{
    return aiRuntimeGeneration.isEnabled() &&
           aiRuntimeGeneration.hasLoadedModel();
}

void MIDIGenGXAudioProcessor::setAIRuntimeEnabled(
    bool enabled) noexcept
{
    aiRuntimeGeneration.setEnabled(
        enabled);
}

bool MIDIGenGXAudioProcessor::isAIRuntimeEnabled()
    const noexcept
{
    return aiRuntimeGeneration.isEnabled();
}

void MIDIGenGXAudioProcessor::setAIRuntimeGenerationProvider(
    midigengx::plugin::AIRuntimeGeneration::GenerationProvider provider)
{
    aiRuntimeGeneration.setProvider(
        std::move(provider));
}

bool MIDIGenGXAudioProcessor::loadAIRuntimeModel(
    const juce::MemoryBlock& artifactBytes)
{
    midigengx::music::CompositionConditionedSequenceNeuralModelArtifact conditionedArtifact;

    conditionedArtifact.bytes.resize(
        artifactBytes.getSize());

    if (artifactBytes.getSize() != 0)
    {
        std::memcpy(
            conditionedArtifact.bytes.data(),
            artifactBytes.getData(),
            artifactBytes.getSize());
    }

    // Phase 123 embeds the conditioned sequence model. Keep the legacy
    // artifact path below so existing external model-loading behavior remains
    // compatible with the pre-conditioned runtime API.
    if (aiRuntimeGeneration.loadConditionedModelArtifact(
            conditionedArtifact))
    {
        return true;
    }

    midigengx::music::CompositionNeuralModelArtifact legacyArtifact;
    legacyArtifact.bytes = conditionedArtifact.bytes;

    return aiRuntimeGeneration.loadModelArtifact(
        legacyArtifact);
}

bool MIDIGenGXAudioProcessor::loadAIRuntimeModelFromFile(
    const juce::File& artifactFile)
{
    if (!artifactFile.existsAsFile())
        return false;

    juce::MemoryBlock bytes;

    if (!artifactFile.loadFileAsData(
            bytes))
    {
        return false;
    }

    return loadAIRuntimeModel(
        bytes);
}

bool MIDIGenGXAudioProcessor::hasLoadedAIRuntimeModel()
    const noexcept
{
    return aiRuntimeGeneration.hasLoadedModel();
}

void MIDIGenGXAudioProcessor::clearAIRuntimeModel()
{
    aiRuntimeGeneration.clearModel();
}

bool MIDIGenGXAudioProcessor::isTransportPlaying() const noexcept
{
    const auto position =
        getPositionInfo();

    return position.hasValue() &&
           position->getIsPlaying();
}

void MIDIGenGXAudioProcessor::setKey(
    int pitchClass) noexcept
{
    keyPitchClass.store(
        ((pitchClass % 12) + 12) % 12);

    markContextChanged();
}

int MIDIGenGXAudioProcessor::getKey() const noexcept
{
    return keyPitchClass.load();
}

void MIDIGenGXAudioProcessor::setScale(
    int newScaleType) noexcept
{
    scaleType.store(newScaleType);
    markContextChanged();
}

int MIDIGenGXAudioProcessor::getScale() const noexcept
{
    return scaleType.load();
}

void MIDIGenGXAudioProcessor::setRole(
    int newRole) noexcept
{
    roleType.store(newRole);
    markContextChanged();
}

int MIDIGenGXAudioProcessor::getRole() const noexcept
{
    return roleType.load();
}

void MIDIGenGXAudioProcessor::setLengthBars(
    int bars) noexcept
{
    lengthBars.store(
        juce::jlimit(
            1,
            64,
            bars));

    markContextChanged();
}

int MIDIGenGXAudioProcessor::getLengthBars() const noexcept
{
    return lengthBars.load();
}

void MIDIGenGXAudioProcessor::setDensity(
    int value) noexcept
{
    density.store(
        juce::jlimit(
            0,
            100,
            value));

    markContextChanged();
}

int MIDIGenGXAudioProcessor::getDensity() const noexcept
{
    return density.load();
}

void MIDIGenGXAudioProcessor::setVariation(
    int value) noexcept
{
    variation.store(
        juce::jlimit(
            0,
            100,
            value));

    markContextChanged();
}

int MIDIGenGXAudioProcessor::getVariation() const noexcept
{
    return variation.load();
}

void MIDIGenGXAudioProcessor::setComplexity(int value) noexcept
{
    complexity.store(juce::jlimit(0, 100, value));
    markContextChanged();
}

int MIDIGenGXAudioProcessor::getComplexity() const noexcept
{
    return complexity.load();
}

void MIDIGenGXAudioProcessor::setSyncopation(int value) noexcept
{
    syncopation.store(juce::jlimit(0, 100, value));
    markContextChanged();
}

int MIDIGenGXAudioProcessor::getSyncopation() const noexcept
{
    return syncopation.load();
}

void MIDIGenGXAudioProcessor::setTension(int value) noexcept
{
    tension.store(juce::jlimit(0, 100, value));
    markContextChanged();
}

int MIDIGenGXAudioProcessor::getTension() const noexcept
{
    return tension.load();
}

void MIDIGenGXAudioProcessor::setRepetition(int value) noexcept
{
    repetition.store(juce::jlimit(0, 100, value));
    markContextChanged();
}

int MIDIGenGXAudioProcessor::getRepetition() const noexcept
{
    return repetition.load();
}

void MIDIGenGXAudioProcessor::setHumanization(
    int value) noexcept
{
    humanization.store(
        juce::jlimit(0, 100, value));

    markContextChanged();
}

int MIDIGenGXAudioProcessor::getHumanization() const noexcept
{
    return humanization.load();
}

void MIDIGenGXAudioProcessor::setNoteLengthVariation(
    int value) noexcept
{
    noteLengthVariation.store(
        juce::jlimit(0, 100, value));

    markContextChanged();
}

int MIDIGenGXAudioProcessor::getNoteLengthVariation() const noexcept
{
    return noteLengthVariation.load();
}

void MIDIGenGXAudioProcessor::setNoteLength(
    int length) noexcept
{
    noteLength.store(
        juce::jlimit(
            0,
            static_cast<int>(
                midigengx::domain::NoteLength::Custom),
            length));

    markContextChanged();
}

int MIDIGenGXAudioProcessor::getNoteLength() const noexcept
{
    return noteLength.load();
}

void MIDIGenGXAudioProcessor::setPhraseContour(
    int contour) noexcept
{
    phraseContour.store(
        juce::jlimit(
            0,
            static_cast<int>(
                midigengx::domain::PhraseContour::Custom),
            contour));

    markContextChanged();
}

int MIDIGenGXAudioProcessor::getPhraseContour() const noexcept
{
    return phraseContour.load();
}

void MIDIGenGXAudioProcessor::setCadenceStyle(
    int cadence) noexcept
{
    cadenceStyle.store(
        juce::jlimit(
            0,
            static_cast<int>(
                midigengx::domain::CadenceStyle::Custom),
            cadence));

    markContextChanged();
}

int MIDIGenGXAudioProcessor::getCadenceStyle() const noexcept
{
    return cadenceStyle.load();
}

void MIDIGenGXAudioProcessor::setCadenceStrength(
    int value) noexcept
{
    cadenceStrength.store(
        juce::jlimit(
            0,
            100,
            value));

    markContextChanged();
}

int MIDIGenGXAudioProcessor::getCadenceStrength() const noexcept
{
    return cadenceStrength.load();
}

void MIDIGenGXAudioProcessor::setGenrePreset(
    int genre) noexcept
{
    genrePreset.store(
        juce::jlimit(
            0,
            static_cast<int>(
                midigengx::domain::GenrePreset::Count) - 1,
            genre));

    markContextChanged();
}

int MIDIGenGXAudioProcessor::getGenrePreset() const noexcept
{
    return genrePreset.load();
}

void MIDIGenGXAudioProcessor::applyGenrePreset(
    int genre) noexcept
{
    const int safeGenre =
        juce::jlimit(
            0,
            static_cast<int>(
                midigengx::domain::GenrePreset::Count) - 1,
            genre);

    const auto preset =
        midigengx::domain::getGenrePresetValues(
            static_cast<midigengx::domain::GenrePreset>(
                safeGenre));

    genrePreset.store(safeGenre);

    density.store(preset.density);
    variation.store(preset.variation);
    complexity.store(preset.complexity);
    syncopation.store(preset.syncopation);
    tension.store(preset.tension);
    repetition.store(preset.repetition);
    humanization.store(preset.humanization);
    noteLengthVariation.store(
        preset.noteLengthVariation);
    cadenceStrength.store(
        preset.cadenceStrength);

    noteLength.store(
        static_cast<int>(
            preset.noteLength));

    phraseContour.store(
        static_cast<int>(
            preset.phraseContour));

    cadenceStyle.store(
        static_cast<int>(
            preset.cadenceStyle));

    markContextChanged();
}

void MIDIGenGXAudioProcessor::setOctaveLow(
    int octave) noexcept
{
    octaveLow.store(
        juce::jlimit(
            -8,
            8,
            octave));

    markContextChanged();
}

int MIDIGenGXAudioProcessor::getOctaveLow() const noexcept
{
    return octaveLow.load();
}

void MIDIGenGXAudioProcessor::setOctaveHigh(
    int octave) noexcept
{
    octaveHigh.store(
        juce::jlimit(
            -8,
            8,
            octave));

    markContextChanged();
}

int MIDIGenGXAudioProcessor::getOctaveHigh() const noexcept
{
    return octaveHigh.load();
}

void MIDIGenGXAudioProcessor::setOctaveShift(
    int shift) noexcept
{
    octaveShift.store(
        juce::jlimit(
            -2,
            2,
            shift));

    markContextChanged();
}

int MIDIGenGXAudioProcessor::getOctaveShift() const noexcept
{
    return octaveShift.load();
}

void MIDIGenGXAudioProcessor::requestGeneration() noexcept
{
    requestPhraseGeneration();
}

bool MIDIGenGXAudioProcessor::isGenerationPending() const noexcept
{
    return generationPending.load();
}

juce::AudioProcessorEditor*
MIDIGenGXAudioProcessor::createEditor()
{
    return new MIDIGenGXAudioProcessorEditor(*this);
}

bool MIDIGenGXAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String
MIDIGenGXAudioProcessor::getName() const
{
    return "MIDI-GenGX";
}

bool MIDIGenGXAudioProcessor::acceptsMidi() const
{
    return true;
}

bool MIDIGenGXAudioProcessor::producesMidi() const
{
    return true;
}

bool MIDIGenGXAudioProcessor::isMidiEffect() const
{
    return false;
}

double MIDIGenGXAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MIDIGenGXAudioProcessor::getNumPrograms()
{
    return 1;
}

int MIDIGenGXAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MIDIGenGXAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String
MIDIGenGXAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void MIDIGenGXAudioProcessor::changeProgramName(
    int index,
    const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void MIDIGenGXAudioProcessor::getStateInformation(
    juce::MemoryBlock& destData)
{
    juce::ValueTree state(
        "MIDI-GenGXState");

    state.setProperty(
        generatorEnabledKey,
        generatorEnabled.load(),
        nullptr);

    state.setProperty(
        keyKey,
        getKey(),
        nullptr);

    state.setProperty(
        scaleKey,
        getScale(),
        nullptr);

    state.setProperty(
        roleKey,
        getRole(),
        nullptr);

    state.setProperty(
        lengthKey,
        getLengthBars(),
        nullptr);

    state.setProperty(
        densityKey,
        getDensity(),
        nullptr);

    state.setProperty(
        variationKey,
        getVariation(),
        nullptr);

    state.setProperty(
        complexityKey,
        getComplexity(),
        nullptr);

    state.setProperty(
        syncopationKey,
        getSyncopation(),
        nullptr);

    state.setProperty(
        tensionKey,
        getTension(),
        nullptr);

    state.setProperty(
        repetitionKey,
        getRepetition(),
        nullptr);

    state.setProperty(
        humanizationKey,
        getHumanization(),
        nullptr);

    state.setProperty(
        noteLengthVariationKey,
        getNoteLengthVariation(),
        nullptr);

    state.setProperty(
        noteLengthKey,
        getNoteLength(),
        nullptr);

    state.setProperty(
        phraseContourKey,
        getPhraseContour(),
        nullptr);

    state.setProperty(
        cadenceStyleKey,
        getCadenceStyle(),
        nullptr);

    state.setProperty(
        cadenceStrengthKey,
        getCadenceStrength(),
        nullptr);

    state.setProperty(
        genrePresetKey,
        getGenrePreset(),
        nullptr);

    state.setProperty(
        octaveLowKey,
        getOctaveLow(),
        nullptr);

    state.setProperty(
        octaveHighKey,
        getOctaveHigh(),
        nullptr);

    state.setProperty(
        octaveShiftKey,
        getOctaveShift(),
        nullptr);

    std::unique_ptr<juce::XmlElement> xml(
        state.createXml());

    copyXmlToBinary(
        *xml,
        destData);
}

void MIDIGenGXAudioProcessor::setStateInformation(
    const void* data,
    int sizeInBytes)
{
    if (auto xml =
            getXmlFromBinary(
                data,
                sizeInBytes))
    {
        if (!xml->hasTagName(
                "MIDI-GenGXState"))
        {
            return;
        }

        const auto state =
            juce::ValueTree::fromXml(*xml);

        if (!state.isValid())
            return;

        const bool wasGeneratorEnabled =
            generatorEnabled.load(std::memory_order_acquire);

        const bool savedGeneratorEnabled =
            static_cast<bool>(
                state.getProperty(
                    generatorEnabledKey,
                    false));

        generatorEnabled.store(
            savedGeneratorEnabled,
            std::memory_order_release);

        restoringState = true;

        setKey(static_cast<int>(
            state.getProperty(
                keyKey,
                0)));

        setScale(static_cast<int>(
            state.getProperty(
                scaleKey,
                static_cast<int>(
                    midigengx::domain::ScaleType::Minor))));

        setRole(static_cast<int>(
            state.getProperty(
                roleKey,
                static_cast<int>(
                    midigengx::domain::Role::Lead))));

        setLengthBars(static_cast<int>(
            state.getProperty(
                lengthKey,
                4)));

        setDensity(static_cast<int>(
            state.getProperty(
                densityKey,
                50)));

        setVariation(static_cast<int>(
            state.getProperty(
                variationKey,
                25)));

        setComplexity(static_cast<int>(
            state.getProperty(
                complexityKey,
                50)));

        setSyncopation(static_cast<int>(
            state.getProperty(
                syncopationKey,
                50)));

        setTension(static_cast<int>(
            state.getProperty(
                tensionKey,
                50)));

        setRepetition(static_cast<int>(
            state.getProperty(
                repetitionKey,
                50)));

        setHumanization(static_cast<int>(
            state.getProperty(
                humanizationKey,
                0)));

        setNoteLengthVariation(static_cast<int>(
            state.getProperty(
                noteLengthVariationKey,
                0)));

        setNoteLength(static_cast<int>(
            state.getProperty(
                noteLengthKey,
                static_cast<int>(
                    midigengx::domain::NoteLength::Auto))));

        setPhraseContour(static_cast<int>(
            state.getProperty(
                phraseContourKey,
                static_cast<int>(
                    midigengx::domain::PhraseContour::Arch))));

        setCadenceStyle(static_cast<int>(
            state.getProperty(
                cadenceStyleKey,
                static_cast<int>(
                    midigengx::domain::CadenceStyle::Root))));

        setCadenceStrength(static_cast<int>(
            state.getProperty(
                cadenceStrengthKey,
                75)));

        setGenrePreset(static_cast<int>(
            state.getProperty(
                genrePresetKey,
                static_cast<int>(
                    midigengx::domain::GenrePreset::Custom))));

        setOctaveLow(static_cast<int>(
            state.getProperty(
                octaveLowKey,
                2)));

        setOctaveHigh(static_cast<int>(
            state.getProperty(
                octaveHighKey,
                4)));

        setOctaveShift(static_cast<int>(
            state.getProperty(
                octaveShiftKey,
                0)));

        restoringState = false;

        generationRequestId.fetch_add(1, std::memory_order_acq_rel);
        if (generationWorker)
            generationWorker->invalidate();

        generationPending.store(
            savedGeneratorEnabled,
            std::memory_order_release);

        stopGenerationRequested.store(
            wasGeneratorEnabled || savedGeneratorEnabled,
            std::memory_order_release);

        if (savedGeneratorEnabled)
            requestPhraseGeneration();
    }
}

juce::AudioProcessor*
JUCE_CALLTYPE createPluginFilter()
{
    return new MIDIGenGXAudioProcessor();
}
