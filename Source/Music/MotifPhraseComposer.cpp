#include "MotifPhraseComposer.h"
#include "PhraseStructure.h"
#include "PhraseDevelopmentPlan.h"
#include "HarmonyPlan.h"
#include "HarmonyGuidance.h"
#include "RhythmPlan.h"
#include "MelodicMotionGuidance.h"

#include "MotifDevelopment.h"
#include "../Domain/Key.h"

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

namespace midigengx::music
{
namespace
{

constexpr double kEpsilon = 1.0e-9;

int normalizePitchClass(int midi) noexcept
{
    const int pc = midi % 12;
    return pc < 0 ? pc + 12 : pc;
}

int lowestMidiForOctave(int octave) noexcept
{
    return std::clamp(
        12 * (octave + 1),
        0,
        127);
}

std::vector<int> buildScalePool(
    const std::vector<int>& pitchClasses,
    int lowMidi,
    int highMidi)
{
    lowMidi = std::min(lowMidi, highMidi);
    highMidi = std::max(lowMidi, highMidi);

    std::vector<int> pool;

    for (int midi = lowMidi; midi <= highMidi; ++midi)
    {
        if (std::find(
                pitchClasses.begin(),
                pitchClasses.end(),
                normalizePitchClass(midi)) != pitchClasses.end())
        {
            pool.push_back(midi);
        }
    }

    return pool;
}

int nearestScalePitch(
    const std::vector<int>& pitchClasses,
    int targetMidi,
    int lowMidi,
    int highMidi)
{
    int best = lowMidi;
    int bestDistance = 1000000;

    for (int midi = lowMidi; midi <= highMidi; ++midi)
    {
        if (std::find(
                pitchClasses.begin(),
                pitchClasses.end(),
                normalizePitchClass(midi)) == pitchClasses.end())
        {
            continue;
        }

        const int distance =
            std::abs(midi - targetMidi);

        if (distance < bestDistance)
        {
            bestDistance = distance;
            best = midi;
        }
    }

    return best;
}

} // namespace

Phrase MotifPhraseComposer::compose(
    const Motif& seedMotif,
    const midigengx::domain::MusicalContext& inputContext,
    std::uint32_t) noexcept
{
    Phrase phrase;

    if (!seedMotif.isValid())
        return phrase;

    auto context = inputContext;
    context.normalize();

    const int rootPitchClass =
        midigengx::domain::toPitchClass(context.key);

    const auto pitchClasses =
        context.scale.getPitchClasses(rootPitchClass);

    if (pitchClasses.empty())
        return phrase;

    const int lowMidi =
        lowestMidiForOctave(context.parameters.octaveLow);

    const int highMidi =
        lowestMidiForOctave(context.parameters.octaveHigh);

    const auto pitchPool =
        buildScalePool(
            pitchClasses,
            lowMidi,
            highMidi);

    if (pitchPool.empty())
        return phrase;

    const int phraseBars =
        std::max(
            1,
            context.parameters.phraseLengthBars);

    const int totalBars =
        std::max(
            phraseBars,
            context.parameters.lengthBars);

    const double phraseLength =
        static_cast<double>(phraseBars) * 4.0;

    const int phraseCount =
        std::max(
            1,
            (totalBars + phraseBars - 1) /
                phraseBars);

    const auto structure =
        planPhraseStructure(context);

    const auto developmentPlan =
        planPhraseDevelopment(
            context,
            structure);

    if (!developmentPlan.isValid() ||
        developmentPlan.sections.size() !=
            structure.sections.size())
    {
        return phrase;
    }

    const auto harmony =
        planHarmony(
            context,
            structure);

    if (!harmony.isValid())
        return phrase;

    const auto rhythm =
        planRhythm(context);

    if (!rhythm.isValid())
        return phrase;


    for (int phraseIndex = 0;
         phraseIndex < phraseCount;
         ++phraseIndex)
    {
        Motif developed =
            seedMotif;

        const auto& development =
            developmentPlan.sections[
                static_cast<std::size_t>(
                    phraseIndex)];

        if (development.role !=
            PhraseDevelopmentRole::Statement)
        {
            if (development.motifVariationAmount > 0)
            {
                developed =
                    MotifDevelopment::varyIntervals(
                        developed,
                        development.motifVariationAmount);

                if (!developed.isValid())
                    continue;
            }

            developed =
                MotifDevelopment::transpose(
                    developed,
                    development.transpositionSemitones);

            if (!developed.isValid())
                continue;

            if (development.timeFactor != 1.0)
            {
                developed =
                    MotifDevelopment::stretchTime(
                        developed,
                        development.timeFactor);

                if (!developed.isValid())
                    continue;
            }

            if (development.retrogradeMotif)
            {
                developed =
                    MotifDevelopment::retrograde(
                        developed);

                if (!developed.isValid())
                    continue;
            }

            if (development.invertMotif)
            {
                developed =
                    MotifDevelopment::invert(
                        developed);

                if (!developed.isValid())
                    continue;
            }

            if (development.sequenceRepetitions > 1)
            {
                developed =
                    MotifDevelopment::sequence(
                        developed,
                        development.sequenceRepetitions,
                        development.sequenceStepSemitones);

                if (!developed.isValid())
                    continue;

                // Keep a sequence inside the original phrase slot instead
                // of increasing the composition length.
                developed =
                    MotifDevelopment::stretchTime(
                        developed,
                        1.0 /
                        static_cast<double>(
                            development.sequenceRepetitions));

                if (!developed.isValid())
                    continue;
            }
        }

        if (!developed.isValid())
            continue;

        developed =
            applyRhythmPlan(
                developed,
                rhythm);

        if (!developed.isValid())
            continue;

        const int centerIndex =
            static_cast<int>(pitchPool.size() / 2);

        const int desiredIndex =
            std::clamp(
                centerIndex +
                    context.parameters.octaveShift *
                        std::max(
                            1,
                            static_cast<int>(
                                pitchPool.size() / 6)),
                0,
                static_cast<int>(pitchPool.size()) - 1);

        const int firstPitch =
            pitchPool[
                static_cast<std::size_t>(
                    desiredIndex)];

        auto occurrence =
            applyMotif(
                developed,
                firstPitch,
                static_cast<double>(
                    phraseIndex) *
                    phraseLength,
                1);

        // Snap every absolute occurrence back to the musical constraints.
        // Motif intervals stay intact as much as possible while each resulting
        // note remains in the configured register and selected scale.
        int previousMidiForMotion =
            occurrence.notes.empty()
                ? 60
                : occurrence.notes.front().midiNote;

        bool hasPreviousMotionNote = false;

        for (auto& note : occurrence.notes)
        {
            const int clamped =
                std::clamp(
                    note.midiNote,
                    lowMidi,
                    highMidi);

            const auto* harmonyEvent =
                findHarmonyEventAtBeat(
                    harmony,
                    note.startBeat);

            if (harmonyEvent != nullptr)
            {
                const auto guidance =
                    buildHarmonyGuidance(
                        harmony,
                        *harmonyEvent,
                        pitchClasses,
                        context.parameters.cadenceStrength);

                const double beatInSection =
                    note.startBeat -
                    harmonyEvent->startBeat;

                const int previousMidi =
                    hasPreviousMotionNote
                        ? previousMidiForMotion
                        : clamped;

                note.midiNote =
                    chooseMelodicMotionPitchWithTendency(
                        clamped,
                        previousMidi,
                        lowMidi,
                        highMidi,
                        pitchClasses,
                        rootPitchClass,
                        guidance,
                        context.parameters.complexity,
                        harmonyEvent->tension,
                        context.parameters.cadenceStrength,
                        phraseIndex == phraseCount - 1,
                        beatInSection);
            }
            else
            {
                note.midiNote =
                    nearestScalePitch(
                        pitchClasses,
                        clamped,
                        lowMidi,
                        highMidi);
            }

            previousMidiForMotion =
                note.midiNote;
            hasPreviousMotionNote = true;

            note.velocity =
                std::clamp(
                    note.velocity +
                        (context.parameters.humanization > 0
                            ? static_cast<int>(
                                  std::lround(
                                      context.parameters.humanization *
                                      0.05))
                            : 0),
                    1,
                    127);
        }
        if (phraseIndex == phraseCount - 1 &&
            !occurrence.notes.empty() &&
            context.parameters.cadenceStrength > 0 &&
            structure.isValid())
        {
            const auto& cadence =
                structure.sections.back();

            const int degreeIndex =
                std::clamp(
                    cadence.targetScaleDegree,
                    0,
                    static_cast<int>(pitchClasses.size()) - 1);

            const int targetPc =
                pitchClasses[
                    static_cast<std::size_t>(degreeIndex)];

            auto& finalNote =
                occurrence.notes.back();

            int bestMidi = finalNote.midiNote;
            int bestDistance = 1000000;

            for (int midi = lowMidi;
                 midi <= highMidi;
                 ++midi)
            {
                const int pc =
                    ((midi % 12) + 12) % 12;

                if (pc != targetPc)
                    continue;

                const int distance =
                    std::abs(midi - finalNote.midiNote);

                if (distance < bestDistance)
                {
                    bestDistance = distance;
                    bestMidi = midi;
                }
            }

            const int strength =
                std::clamp(
                    context.parameters.cadenceStrength,
                    0,
                    100);

            if (strength >= 50)
            {
                finalNote.midiNote = bestMidi;
            }
            else if (strength > 0)
            {
                finalNote.midiNote =
                    nearestScalePitch(
                        pitchClasses,
                        static_cast<int>(
                            std::lround(
                                finalNote.midiNote * 0.5 +
                                bestMidi * 0.5)),
                        lowMidi,
                        highMidi);
            }
        }



        phrase.notes.insert(
            phrase.notes.end(),
            occurrence.notes.begin(),
            occurrence.notes.end());
    }

    phrase.lengthBeats =
        static_cast<double>(totalBars) * 4.0;

    phrase.normalize();

    return phrase;
}

} // namespace midigengx::music
