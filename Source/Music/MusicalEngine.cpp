#include "MusicalEngine.h"
#include "MelodicContour.h"

#include "../Domain/Key.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>
#include <utility>

namespace midigengx::music
{
namespace
{

constexpr double kSixteenth = 0.25;
constexpr double kEpsilon = 1.0e-9;

int clampPercent(int value) noexcept
{
    return std::clamp(value, 0, 100);
}

int cadenceScaleIndex(
    midigengx::domain::CadenceStyle style,
    int scaleSize)
{
    if (scaleSize <= 0)
        return 0;

    switch (style)
    {
        case midigengx::domain::CadenceStyle::Root:
            return 0;

        case midigengx::domain::CadenceStyle::Third:
            return std::min(2, scaleSize - 1);

        case midigengx::domain::CadenceStyle::Fifth:
            return std::min(4, scaleSize - 1);

        case midigengx::domain::CadenceStyle::Open:
            return std::min(
                scaleSize - 1,
                std::max(1, scaleSize / 2));

        case midigengx::domain::CadenceStyle::Custom:
            return 0;
    }

    return 0;
}

int chooseNearestPitchClass(
    int pitchClass,
    int targetMidiNote,
    int lowMidi,
    int highMidi)
{
    int best = lowMidi;
    int bestDistance = 1000;

    pitchClass =
        ((pitchClass % 12) + 12) % 12;

    for (int midi = lowMidi;
         midi <= highMidi;
         ++midi)
    {
        if ((midi % 12 + 12) % 12 != pitchClass)
            continue;

        const int distance =
            std::abs(midi - targetMidiNote);

        if (distance < bestDistance)
        {
            best = midi;
            bestDistance = distance;
        }
    }

    return std::clamp(best, lowMidi, highMidi);
}

int weightedStepScore(
    int step,
    int stepsPerBar,
    int syncopation)
{
    // Sixteenth grid in 4/4.
    // Downbeats remain structurally strong while higher syncopation
    // increases the weight of off-beat positions.
    const bool strongBeat = (step % 4) == 0;
    const bool eighthOffbeat = (step % 2) == 1;

    int score = 100;

    if (strongBeat)
        score -= syncopation / 3;
    else if (eighthOffbeat)
        score += syncopation / 2;
    else
        score += syncopation / 5;

    // Keep the first step attractive enough to establish a phrase.
    if (step == 0)
        score += 60;

    // Scale if someone ever changes the grid resolution.
    if (stepsPerBar <= 0)
        return 1;

    return std::max(1, score);
}

} // namespace

double MusicalEngine::beatsPerBar() noexcept
{
    return 4.0;
}

int MusicalEngine::clampMidi(int value) noexcept
{
    return std::clamp(value, 0, 127);
}

int MusicalEngine::lowestMidiForOctaveRange(
    int octaveOffset) noexcept
{
    // OCTAVE LOW/HIGH are absolute C-octave MIDI boundaries.
    // The selected key is applied by the scale pitch pool.
    return 12 * (octaveOffset + 1);
}

int MusicalEngine::chooseNearestScalePitch(
    const std::vector<int>& pitchClasses,
    int targetMidiNote,
    int lowMidi,
    int highMidi)
{
    int best = lowMidi;
    int bestDistance = 1000;

    for (int midi = lowMidi; midi <= highMidi; ++midi)
    {
        const auto pitchClass = midi % 12;

        if (std::find(
                pitchClasses.begin(),
                pitchClasses.end(),
                pitchClass) == pitchClasses.end())
        {
            continue;
        }

        const int distance =
            std::abs(midi - targetMidiNote);

        if (distance < bestDistance)
        {
            best = midi;
            bestDistance = distance;
        }
    }

    return clampMidi(best);
}

std::vector<int> MusicalEngine::buildScalePitchPool(
    const std::vector<int>& pitchClasses,
    int lowMidi,
    int highMidi)
{
    const int safeLow = std::min(lowMidi, highMidi);
    const int safeHigh = std::max(lowMidi, highMidi);

    std::vector<int> pool;

    for (int midi = safeLow; midi <= safeHigh; ++midi)
    {
        const int pitchClass =
            ((midi % 12) + 12) % 12;

        if (std::find(
                pitchClasses.begin(),
                pitchClasses.end(),
                pitchClass) != pitchClasses.end())
        {
            pool.push_back(midi);
        }
    }

    return pool;
}

int MusicalEngine::choosePitchAcrossRange(
    const std::vector<int>& pitchPool,
    int targetMidiNote,
    int desiredRegisterIndex)
{
    if (pitchPool.empty())
        return clampMidi(targetMidiNote);

    const int index =
        std::clamp(
            desiredRegisterIndex,
            0,
            static_cast<int>(pitchPool.size()) - 1);

    const int registerTarget =
        pitchPool[static_cast<std::size_t>(index)];

    int best = pitchPool.front();

    int bestDistance =
        std::abs(best - targetMidiNote) +
        std::abs(best - registerTarget);

    for (const int midi : pitchPool)
    {
        const int distance =
            std::abs(midi - targetMidiNote) +
            std::abs(midi - registerTarget);

        if (distance < bestDistance)
        {
            best = midi;
            bestDistance = distance;
        }
    }

    return best;
}

std::vector<int> MusicalEngine::selectOnsets(
    int stepsPerBar,
    int count,
    int syncopation,
    std::mt19937& rng)
{
    stepsPerBar = std::max(1, stepsPerBar);
    count = std::clamp(count, 1, stepsPerBar);
    syncopation = clampPercent(syncopation);

    std::vector<int> remaining(stepsPerBar);
    std::iota(remaining.begin(), remaining.end(), 0);

    std::vector<int> selected;
    selected.reserve(static_cast<std::size_t>(count));

    for (int i = 0; i < count; ++i)
    {
        int totalWeight = 0;

        for (const auto step : remaining)
            totalWeight += weightedStepScore(
                step,
                stepsPerBar,
                syncopation);

        std::uniform_int_distribution<int> pick(
            1,
            std::max(1, totalWeight));

        int target = pick(rng);
        int chosenIndex = 0;

        for (std::size_t j = 0; j < remaining.size(); ++j)
        {
            target -= weightedStepScore(
                remaining[j],
                stepsPerBar,
                syncopation);

            if (target <= 0)
            {
                chosenIndex = static_cast<int>(j);
                break;
            }
        }

        selected.push_back(remaining[
            static_cast<std::size_t>(chosenIndex)]);

        remaining.erase(
            remaining.begin() + chosenIndex);
    }

    std::sort(selected.begin(), selected.end());
    return selected;
}

double MusicalEngine::noteDurationBeats(
    midigengx::domain::NoteLength length,
    double availableBeats)
{
    availableBeats = std::max(0.05, availableBeats);

    double requested = availableBeats * 0.80;

    switch (length)
    {
        case midigengx::domain::NoteLength::Auto:
            requested = availableBeats * 0.80;
            break;

        case midigengx::domain::NoteLength::Short:
            requested = 0.20;
            break;

        case midigengx::domain::NoteLength::Medium:
            requested = 0.50;
            break;

        case midigengx::domain::NoteLength::Long:
            requested = 0.90;
            break;

        case midigengx::domain::NoteLength::Legato:
            requested = availableBeats * 0.96;
            break;

        case midigengx::domain::NoteLength::Staccato:
            requested = 0.12;
            break;

        case midigengx::domain::NoteLength::Custom:
            requested = availableBeats * 0.70;
            break;
    }

    return std::clamp(
        requested,
        0.05,
        std::max(0.05, availableBeats - 0.02));
}

int MusicalEngine::chooseScaleIndexNear(
    int previousScaleIndex,
    int scaleSize,
    int tension,
    int variation,
    std::mt19937& rng)
{
    if (scaleSize <= 0)
        return 0;

    previousScaleIndex =
        std::clamp(previousScaleIndex, 0, scaleSize - 1);

    tension = clampPercent(tension);
    variation = clampPercent(variation);

    const int maxStep =
        tension < 35 ? 1 :
        tension < 70 ? 2 :
        std::min(3, scaleSize - 1);

    std::uniform_int_distribution<int> direction(-1, 1);
    std::uniform_int_distribution<int> amount(0, maxStep);

    int delta = amount(rng);

    if (delta > 0)
        delta *= direction(rng);

    if (variation > 50)
    {
        std::uniform_int_distribution<int> extra(-1, 1);
        delta += extra(rng);
    }

    return std::clamp(
        previousScaleIndex + delta,
        0,
        scaleSize - 1);
}


Phrase MusicalEngine::generate(
    const midigengx::domain::MusicalContext& inputContext,
    std::uint32_t seed) const
{
    switch (inputContext.role)
    {
        case midigengx::domain::Role::Lead:
            return generateLead(inputContext, seed);

        case midigengx::domain::Role::Melody:
            return generateLead(inputContext, seed + 17);

        case midigengx::domain::Role::Pluck:
            return generateMonophonicRole(inputContext, seed + 31, 0, true, false);

        case midigengx::domain::Role::Piano:
            return generateMonophonicRole(inputContext, seed + 43, 0, false, false);

        case midigengx::domain::Role::Sequence:
            return generateMonophonicRole(inputContext, seed + 59, 0, true, false);

        case midigengx::domain::Role::Bass:
            return generateBass(inputContext, seed);

        case midigengx::domain::Role::Arp:
            return generateArp(inputContext, seed);

        case midigengx::domain::Role::Chords:
            return generateChords(inputContext, seed);

        case midigengx::domain::Role::Pad:
            return generatePad(inputContext, seed);
    }

    return generateLead(inputContext, seed);
}

Phrase MusicalEngine::generateWithAIGuidance(
    const midigengx::domain::MusicalContext& inputContext,
    const CompositionAIGuidance& guidance,
    std::uint32_t seed) const
{
    if (!guidance.isValid())
        return generate(
            inputContext,
            seed);

    auto guidedContext =
        inputContext;

    guidedContext.normalize();

    // AI guidance is intentionally a soft modulation layer. It never replaces
    // the user's explicit structural constraints such as key, scale, octave,
    // length or role. The existing MusicalEngine remains the authoritative
    // generator and therefore keeps all existing safety guarantees.

    const auto tensionTarget =
        static_cast<int>(
            std::lround(
                (guidance.tensionTarget + 1.0) *
                50.0));

    const auto tensionDeltaTarget =
        static_cast<int>(
            std::lround(
                std::abs(
                    guidance.tensionDeltaTarget) *
                100.0));

    const auto harmonyTarget =
        static_cast<int>(
            std::lround(
                (guidance.harmonyQualityTarget + 1.0) *
                50.0));

    guidedContext.parameters.tension =
        std::clamp(
            (guidedContext.parameters.tension +
             tensionTarget) /
                2,
            0,
            100);

    guidedContext.parameters.variation =
        std::clamp(
            (guidedContext.parameters.variation +
             tensionDeltaTarget) /
                2,
            0,
            100);

    guidedContext.parameters.complexity =
        std::clamp(
            (guidedContext.parameters.complexity +
             harmonyTarget) /
                2,
            0,
            100);

    const auto generated =
        generate(
            guidedContext,
            seed);

    return generated;
}

Phrase MusicalEngine::generateMonophonicRole(
    const midigengx::domain::MusicalContext& inputContext,
    std::uint32_t seed,
    int octaveShift,
    bool fasterRhythm,
    bool emphasizeRoot) const
{
    auto context = inputContext;
    context.normalize();

    if (fasterRhythm)
        context.parameters.density = std::min(100, context.parameters.density + 15);

    auto phrase = generateLead(context, seed);

    const int rootPc = midigengx::domain::toPitchClass(context.key);
    const auto scale = context.scale.getPitchClasses(rootPc);

    if (scale.empty())
        return phrase;

    const int lowMidi = clampMidi(
        lowestMidiForOctaveRange(context.parameters.octaveLow));

    const int highMidi = clampMidi(
        lowestMidiForOctaveRange(context.parameters.octaveHigh));

    for (auto& note : phrase.notes)
    {
        int target = note.midiNote + octaveShift * 12;

        if (emphasizeRoot && (std::fmod(note.startBeat, 1.0) < 1.0e-9))
            target = chooseNearestScalePitch(scale, lowMidi + rootPc, lowMidi, highMidi);

        note.midiNote = chooseNearestScalePitch(
            scale,
            std::clamp(target, lowMidi, highMidi),
            lowMidi,
            highMidi);

        if (fasterRhythm)
            note.durationBeats = std::min(note.durationBeats, 0.35);
    }

    phrase.normalize();
    return phrase;
}

Phrase MusicalEngine::generateBass(
    const midigengx::domain::MusicalContext& inputContext,
    std::uint32_t seed) const
{
    auto context = inputContext;
    context.normalize();

    context.parameters.octaveLow = std::max(-2, context.parameters.octaveLow - 2);
    context.parameters.octaveHigh = std::min(0, context.parameters.octaveHigh - 1);
    context.parameters.density = std::min(context.parameters.density, 55);
    context.parameters.syncopation = std::min(context.parameters.syncopation, 65);

    if (context.parameters.noteLength ==
        midigengx::domain::NoteLength::Auto)
    {
        context.parameters.noteLength =
            midigengx::domain::NoteLength::Long;
    }

    context.parameters.catchiness =
        std::max(context.parameters.catchiness, 65);

    auto phrase = generateLead(context, seed);
    const int rootPc = midigengx::domain::toPitchClass(context.key);
    const auto scale = context.scale.getPitchClasses(rootPc);

    if (!scale.empty())
    {
        const int lowMidi = clampMidi(lowestMidiForOctaveRange(context.parameters.octaveLow));
        const int highMidi = clampMidi(lowestMidiForOctaveRange(context.parameters.octaveHigh));
        const int rootTarget = chooseNearestScalePitch(scale, rootPc + 12 * 1, lowMidi, highMidi);

        for (std::size_t i = 0; i < phrase.notes.size(); ++i)
        {
            auto& note = phrase.notes[i];
            const bool strong = std::fmod(note.startBeat, 1.0) < 1.0e-9;
            const int target = strong ? rootTarget : note.midiNote;
            note.midiNote = chooseNearestScalePitch(scale, target, lowMidi, highMidi);
            note.durationBeats = std::max(note.durationBeats, 0.65);
        }
    }

    phrase.normalize();
    return phrase;
}

Phrase MusicalEngine::generateArp(
    const midigengx::domain::MusicalContext& inputContext,
    std::uint32_t seed) const
{
    auto context = inputContext;
    context.normalize();
    context.parameters.density =
        std::max(context.parameters.density, 65);

    context.parameters.syncopation =
        std::min(
            100,
            context.parameters.syncopation + 15);

    if (context.parameters.noteLength ==
        midigengx::domain::NoteLength::Auto)
    {
        context.parameters.noteLength =
            midigengx::domain::NoteLength::Short;
    }

    auto phrase = generateLead(context, seed + 101);

    const int rootPc = midigengx::domain::toPitchClass(context.key);
    const auto scale = context.scale.getPitchClasses(rootPc);
    if (scale.empty())
        return phrase;

    std::vector<NoteEvent> arp;
    arp.reserve(phrase.notes.size());

    const int lowMidi = clampMidi(lowestMidiForOctaveRange(context.parameters.octaveLow));
    const int highMidi = clampMidi(lowestMidiForOctaveRange(context.parameters.octaveHigh));

    std::size_t index = 0;
    for (auto note : phrase.notes)
    {
        const int direction = (index % 2 == 0) ? 1 : -1;
        const int scaleIndex = static_cast<int>((index * 2 + (direction < 0 ? 1 : 0)) % scale.size());
        const int pitch = chooseNearestScalePitch(
            scale,
            lowMidi + scale[static_cast<std::size_t>(scaleIndex)],
            lowMidi,
            highMidi);

        note.midiNote = pitch;
        note.durationBeats = 0.18;
        arp.push_back(note);
        ++index;
    }

    phrase.notes = std::move(arp);
    phrase.normalize();
    return phrase;
}

Phrase MusicalEngine::generateChords(
    const midigengx::domain::MusicalContext& inputContext,
    std::uint32_t seed) const
{
    auto context = inputContext;
    context.normalize();
    context.parameters.density =
        std::min(context.parameters.density, 35);

    if (context.parameters.noteLength ==
        midigengx::domain::NoteLength::Auto)
    {
        context.parameters.noteLength =
            midigengx::domain::NoteLength::Long;
    }

    auto source = generateLead(context, seed + 211);
    Phrase phrase;

    const int rootPc = midigengx::domain::toPitchClass(context.key);
    const auto scale = context.scale.getPitchClasses(rootPc);

    if (scale.empty())
        return phrase;

    const int lowMidi = clampMidi(lowestMidiForOctaveRange(context.parameters.octaveLow));
    const int highMidi = clampMidi(lowestMidiForOctaveRange(context.parameters.octaveHigh));

    for (std::size_t i = 0; i < source.notes.size(); ++i)
    {
        const auto& sourceNote = source.notes[i];
        const int rootIndex = static_cast<int>((i % scale.size()));
        const int thirdIndex = (rootIndex + 2) % static_cast<int>(scale.size());
        const int fifthIndex = (rootIndex + 4) % static_cast<int>(scale.size());

        const int pitches[3]{
            scale[static_cast<std::size_t>(rootIndex)],
            scale[static_cast<std::size_t>(thirdIndex)],
            scale[static_cast<std::size_t>(fifthIndex)]
        };

        for (int chordVoice = 0; chordVoice < 3; ++chordVoice)
        {
            NoteEvent note = sourceNote;
            note.midiNote = chooseNearestScalePitch(
                scale,
                lowMidi + pitches[chordVoice] + chordVoice * 12,
                lowMidi,
                highMidi);
            note.durationBeats = std::max(0.7, sourceNote.durationBeats);
            note.velocity = std::clamp(sourceNote.velocity - chordVoice * 8, 1, 127);
            phrase.notes.push_back(note);
        }
    }

    phrase.lengthBeats = source.lengthBeats;
    phrase.normalize();
    return phrase;
}

Phrase MusicalEngine::generatePad(
    const midigengx::domain::MusicalContext& inputContext,
    std::uint32_t seed) const
{
    auto context = inputContext;
    context.normalize();
    context.parameters.density = 20;

    if (context.parameters.noteLength ==
        midigengx::domain::NoteLength::Auto)
    {
        context.parameters.noteLength =
            midigengx::domain::NoteLength::Legato;
    }

    auto phrase = generateChords(context, seed + 307);

    for (auto& note : phrase.notes)
    {
        note.durationBeats = std::min(
            4.0,
            std::max(2.5, note.durationBeats));
        note.velocity = std::max(48, note.velocity - 12);
    }

    phrase.normalize();
    return phrase;
}

Phrase MusicalEngine::generateLead(
    const midigengx::domain::MusicalContext& inputContext,
    std::uint32_t seed) const
{
    auto context = inputContext;
    context.normalize();

    Phrase phrase;

    const int rootPitchClass =
        midigengx::domain::toPitchClass(context.key);

    const auto pitchClasses =
        context.scale.getPitchClasses(rootPitchClass);

    if (pitchClasses.empty())
        return phrase;

    const int lowMidi = clampMidi(
        lowestMidiForOctaveRange(
            context.parameters.octaveLow));

    const int highMidi = clampMidi(
        lowestMidiForOctaveRange(context.parameters.octaveHigh));

    const int safeLow = std::min(lowMidi, highMidi);
    const int safeHigh = std::max(lowMidi, highMidi);

    const auto pitchPool =
        buildScalePitchPool(
            pitchClasses,
            safeLow,
            safeHigh);

    if (pitchPool.empty())
        return phrase;

    const int bars = context.parameters.lengthBars;
    const int phraseBars = context.parameters.phraseLengthBars;
    const int stepsPerBar = 16;

    std::mt19937 rng(seed);

    const auto& params = context.parameters;

    std::uniform_int_distribution<int> humanRoll(
        -100,
        100);

    // Density is mapped to an intentional note count, not just a random
    // probability, so the engine is predictable and testable.
    const int eventsPerBar =
        std::clamp(
            2 + (params.density * 14) / 100,
            1,
            stepsPerBar);

    const int phraseSteps =
        std::max(stepsPerBar,
                 phraseBars * stepsPerBar);

    const int phraseCount =
        std::max(1, (bars + phraseBars - 1) / phraseBars);

    std::vector<int> motifSteps;
    motifSteps.reserve(
        static_cast<std::size_t>(eventsPerBar * phraseBars));

    for (int bar = 0; bar < phraseBars; ++bar)
    {
        const auto local =
            selectOnsets(
                stepsPerBar,
                eventsPerBar,
                params.syncopation,
                rng);

        for (const auto step : local)
            motifSteps.push_back(
                bar * stepsPerBar + step);
    }

    if (motifSteps.empty())
        motifSteps.push_back(0);

    const int scaleSize =
        static_cast<int>(pitchClasses.size());

    std::uniform_int_distribution<int> velocityVariation(
        -8,
        8);

    std::uniform_int_distribution<int> randomScale(
        0,
        scaleSize - 1);

    int previousScaleIndex =
        0;

    std::vector<int> motifScaleIndices(
        motifSteps.size(),
        0);

    std::uniform_int_distribution<int> percentageRoll(
        0,
        99);

    for (int phraseIndex = 0;
         phraseIndex < phraseCount;
         ++phraseIndex)
    {
        const double phraseOffset =
            static_cast<double>(phraseIndex * phraseSteps) /
            4.0;

        for (std::size_t motifIndex = 0;
             motifIndex < motifSteps.size();
             ++motifIndex)
        {
            const int localStep =
                motifSteps[motifIndex];

            const double startBeat =
                phraseOffset +
                static_cast<double>(localStep) *
                    kSixteenth;

            if (startBeat >=
                static_cast<double>(bars) * beatsPerBar())
            {
                continue;
            }

            int scaleIndex =
                static_cast<int>(
                    motifIndex %
                    static_cast<std::size_t>(scaleSize));

            if (phraseIndex == 0)
            {
                if (motifIndex > 0 &&
                    params.complexity > 0)
                {
                    const int effectiveTension =
                        std::clamp(
                            params.tension +
                            params.complexity / 2,
                            0,
                            100);

                    const int effectiveVariation =
                        std::clamp(
                            params.variation +
                            params.complexity,
                            0,
                            100);

                    scaleIndex =
                        chooseScaleIndexNear(
                            previousScaleIndex,
                            scaleSize,
                            effectiveTension,
                            effectiveVariation,
                            rng);
                }

                if (params.catchiness >= 70 &&
                    (localStep % 4) == 0)
                {
                    scaleIndex =
                        std::min(
                            scaleSize - 1,
                            static_cast<int>(
                                (motifIndex % 3) * 2));
                }

                motifScaleIndices[motifIndex] =
                    scaleIndex;
            }
            else
            {
                scaleIndex =
                    motifScaleIndices[motifIndex];

                const int changeProbability =
                    100 - params.repetition;

                if (changeProbability > 0 &&
                    percentageRoll(rng) <
                        changeProbability)
                {
                    const int effectiveVariation =
                        std::clamp(
                            params.variation +
                            params.complexity / 2,
                            0,
                            100);

                    scaleIndex =
                        chooseScaleIndexNear(
                            scaleIndex,
                            scaleSize,
                            params.tension,
                            effectiveVariation,
                            rng);
                }
            }

            // Octave movement is a musical contour modifier. It is combined
            // with the user-facing relative register shift but never exceeds
            // the absolute LOW/HIGH MIDI boundaries.
            int movementShift = 0;

            if (params.octaveMovement > 0)
            {
                const int threshold =
                    std::max(10, 100 - params.octaveMovement);

                std::uniform_int_distribution<int> octaveRoll(0, 99);

                if (octaveRoll(rng) >= threshold)
                {
                    std::uniform_int_distribution<int> octaveDirection(-1, 1);
                    movementShift = octaveDirection(rng);
                }
            }

            const double progress =
                motifSteps.size() <= 1
                    ? 0.0
                    : static_cast<double>(motifIndex) /
                      static_cast<double>(motifSteps.size() - 1);

            // The pitch pool already contains only scale-valid MIDI notes
            // across the COMPLETE configured absolute range.
            const double registerShape =
                MelodicContour::registerShape(
                    params.phraseContour,
                    progress);

            int registerIndex =
                static_cast<int>(
                    std::llround(
                        registerShape *
                        static_cast<double>(
                            pitchPool.size() - 1)));

            const int relativeShift =
                std::clamp(
                    params.octaveShift,
                    -2,
                    2);

            const int shiftSteps =
                std::max(
                    1,
                    static_cast<int>(
                        pitchPool.size() / 6));

            registerIndex +=
                (relativeShift + movementShift) *
                shiftSteps;

            registerIndex =
                std::clamp(
                    registerIndex,
                    0,
                    static_cast<int>(
                        pitchPool.size() - 1));

            int targetMidi =
                pitchPool[
                    static_cast<std::size_t>(
                        registerIndex)];

            const int endpointRegisterIndex =
                MelodicContour::endpointRegisterIndex(
                    params.phraseContour,
                    motifIndex,
                    motifSteps.size(),
                    0,
                    static_cast<int>(
                        pitchPool.size() - 1));

            if (endpointRegisterIndex >= 0)
            {
                targetMidi =
                    pitchPool[
                        static_cast<std::size_t>(
                            endpointRegisterIndex)];
            }

            int midi =
                chooseNearestScalePitch(
                    pitchClasses,
                    targetMidi,
                    safeLow,
                    safeHigh);

            const bool isPhraseCadence =
                motifIndex + 1 ==
                motifSteps.size();

            if (isPhraseCadence &&
                params.cadenceStrength > 0)
            {
                const int roll =
                    percentageRoll(rng);

                if (roll < params.cadenceStrength)
                {
                    const int cadenceIndex =
                        cadenceScaleIndex(
                            params.cadenceStyle,
                            scaleSize);

                    const int cadencePitchClass =
                        pitchClasses[
                            static_cast<std::size_t>(
                                cadenceIndex)];

                    midi =
                        chooseNearestPitchClass(
                            cadencePitchClass,
                            targetMidi,
                            safeLow,
                            safeHigh);
                }
            }

            previousScaleIndex = scaleIndex;

            // Determine available duration from the next motif onset.
            double nextBeat =
                startBeat + 0.25;

            if (motifIndex + 1 < motifSteps.size())
            {
                nextBeat =
                    phraseOffset +
                    static_cast<double>(
                        motifSteps[motifIndex + 1]) *
                        kSixteenth;
            }
            else
            {
                nextBeat =
                    phraseOffset +
                    static_cast<double>(phraseSteps) /
                        4.0;
            }

            double available =
                std::max(0.25, nextBeat - startBeat);

            double duration =
                noteDurationBeats(
                    params.noteLength,
                    available);

            if (isPhraseCadence &&
                params.cadenceStrength >= 60)
            {
                duration =
                    std::min(
                        available,
                        duration * 1.20);
            }

            if (params.noteLengthVariation > 0)
            {
                const double amount =
                    (static_cast<double>(
                        params.noteLengthVariation) /
                     100.0) *
                    0.45;

                const double roll =
                    static_cast<double>(
                        humanRoll(rng)) /
                    100.0;

                duration *=
                    1.0 +
                    roll * amount;

                duration =
                    std::clamp(
                        duration,
                        0.05,
                        std::max(
                            0.05,
                            available - 0.02));
            }

            int velocity =
                88 +
                (params.catchiness - 50) / 3 +
                velocityVariation(rng);

            if ((localStep % 4) == 0)
                velocity += 8;

            velocity =
                std::clamp(velocity, 1, 127);

            double humanizedStartBeat =
                startBeat;

            int humanizedVelocity =
                velocity;

            if (params.humanization > 0)
            {
                const double amount =
                    static_cast<double>(
                        params.humanization) /
                    100.0;

                // Maximum timing drift is 25 ms at 120 BPM,
                // scaled by the user's Humanize amount.
                const double maxTimingBeats =
                    0.05 *
                    amount;

                humanizedStartBeat +=
                    (static_cast<double>(
                        humanRoll(rng)) /
                     100.0) *
                    maxTimingBeats;

                humanizedStartBeat =
                    std::clamp(
                        humanizedStartBeat,
                        phraseOffset,
                        phraseOffset +
                            static_cast<double>(
                                phraseSteps) /
                            4.0 -
                            0.001);

                const int velocityDrift =
                    static_cast<int>(
                        std::lround(
                            (static_cast<double>(
                                 humanRoll(rng)) /
                             100.0) *
                            14.0 *
                            amount));

                humanizedVelocity =
                    std::clamp(
                        humanizedVelocity +
                            velocityDrift,
                        1,
                        127);
            }

            NoteEvent note;
            note.midiNote = midi;
            note.velocity = humanizedVelocity;
            note.startBeat = humanizedStartBeat;
            note.durationBeats = duration;
            note.channel = 1;

            phrase.notes.push_back(note);
        }
    }

    phrase.lengthBeats =
        static_cast<double>(bars) * beatsPerBar();

    phrase.normalize();

    return phrase;
}


} // namespace midigengx::music
