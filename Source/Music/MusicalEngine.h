#pragma once

#include "Phrase.h"
#include "CompositionAIEngineBridge.h"
#include "../Domain/MusicalContext.h"

#include <cstdint>
#include <random>
#include <vector>

namespace midigengx::music
{

class MusicalEngine
{
public:
    Phrase generate(
        const midigengx::domain::MusicalContext& context,
        std::uint32_t seed = 1) const;

    Phrase generateWithAIGuidance(
        const midigengx::domain::MusicalContext& context,
        const CompositionAIGuidance& guidance,
        std::uint32_t seed = 1) const;

    Phrase generateLead(
        const midigengx::domain::MusicalContext& context,
        std::uint32_t seed = 1) const;

    Phrase generateBass(
        const midigengx::domain::MusicalContext& context,
        std::uint32_t seed = 1) const;

    Phrase generateArp(
        const midigengx::domain::MusicalContext& context,
        std::uint32_t seed = 1) const;

    Phrase generateChords(
        const midigengx::domain::MusicalContext& context,
        std::uint32_t seed = 1) const;

    Phrase generatePad(
        const midigengx::domain::MusicalContext& context,
        std::uint32_t seed = 1) const;

private:
    static double beatsPerBar() noexcept;
    static int clampMidi(int value) noexcept;

    static int lowestMidiForOctaveRange(
        int octaveOffset) noexcept;

    static int chooseNearestScalePitch(
        const std::vector<int>& pitchClasses,
        int targetMidiNote,
        int lowMidi,
        int highMidi);

    static std::vector<int> buildScalePitchPool(
        const std::vector<int>& pitchClasses,
        int lowMidi,
        int highMidi);

    static int choosePitchAcrossRange(
        const std::vector<int>& pitchPool,
        int targetMidiNote,
        int desiredRegisterIndex);

    static std::vector<int> selectOnsets(
        int stepsPerBar,
        int count,
        int syncopation,
        std::mt19937& rng);

    static double noteDurationBeats(
        midigengx::domain::NoteLength length,
        double availableBeats);

    static int chooseScaleIndexNear(
        int previousScaleIndex,
        int scaleSize,
        int tension,
        int variation,
        std::mt19937& rng);

    Phrase generateMonophonicRole(
        const midigengx::domain::MusicalContext& context,
        std::uint32_t seed,
        int octaveShift,
        bool fasterRhythm,
        bool emphasizeRoot) const;
};

} // namespace midigengx::music
