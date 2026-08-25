#include "Domain/Key.h"
#include "Domain/MusicalContext.h"
#include "Domain/Scale.h"
#include "Music/MusicalEngine.h"
#include "Music/PhraseAnalysis.h"
#include "Music/Motif.h"
#include "Music/MotifDevelopment.h"
#include "Music/MotifPhraseComposer.h"
#include "Music/PhraseStructure.h"
#include "Music/PhraseDevelopmentPlan.h"
#include "Music/PhraseTensionArc.h"
#include "Music/HarmonyPlan.h"
#include "Music/HarmonyGuidance.h"
#include "Music/RhythmPlan.h"
#include "Music/MelodicContour.h"
#include "Music/MelodicMotion.h"
#include "Music/MelodicMotionGuidance.h"
#include "Music/MelodicResolution.h"
#include "Music/MelodicTendency.h"
#include "Domain/GenerationIntent.h"
#include "Domain/AbletonOctaveConvention.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <set>
#include <vector>

using namespace midigengx;
using namespace midigengx::domain;
using namespace midigengx::music;

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

MusicalContext baseContext()
{
    MusicalContext context;
    context.key = Key::C;
    context.scale = Scale{ScaleType::Minor};
    context.parameters.lengthBars = 4;
    context.parameters.phraseLengthBars = 4;
    context.parameters.octaveLow = 0;
    context.parameters.octaveHigh = 1;
    context.parameters.density = 50;
    context.parameters.syncopation = 50;
    context.parameters.variation = 25;
    context.parameters.repetition = 50;
    return context;
}

void testDeterministicGeneration()
{
    auto context = baseContext();
    MusicalEngine engine;

    const auto first = engine.generate(context, 42);
    const auto second = engine.generate(context, 42);

    expect(first.notes.size() == second.notes.size(), "deterministic note count");
    expect(first.lengthBeats == second.lengthBeats, "deterministic phrase length");

    for (std::size_t i = 0; i < first.notes.size(); ++i)
    {
        expect(first.notes[i].midiNote == second.notes[i].midiNote,
               "deterministic pitch");
        expect(first.notes[i].startBeat == second.notes[i].startBeat,
               "deterministic timing");
        expect(first.notes[i].velocity == second.notes[i].velocity,
               "deterministic velocity");
    }
}

void testScaleSafetyAndRange()
{
    auto context = baseContext();
    context.parameters.octaveLow = -1;
    context.parameters.octaveHigh = 2;

    MusicalEngine engine;
    const auto phrase = engine.generate(context, 123);

    const auto allowed = context.scale.getPitchClasses(toPitchClass(context.key));

    for (const auto& note : phrase.notes)
    {
        expect(note.midiNote >= 0 && note.midiNote <= 127, "MIDI range");
        expect(note.velocity >= 1 && note.velocity <= 127, "velocity range");
        expect(note.startBeat >= 0.0, "non-negative time");
        expect(note.durationBeats > 0.0, "positive duration");
        expect(std::find(allowed.begin(), allowed.end(), note.midiNote % 12) != allowed.end(),
               "note belongs to selected scale");
    }
}

void testDensityControlsNoteCount()
{
    auto low = baseContext();
    auto high = low;
    low.parameters.density = 10;
    high.parameters.density = 90;

    MusicalEngine engine;

    const auto lowPhrase = engine.generate(low, 77);
    const auto highPhrase = engine.generate(high, 77);

    expect(highPhrase.notes.size() > lowPhrase.notes.size(),
           "higher density produces more notes");
}

void testSyncopationChangesRhythmicPlacement()
{
    auto straight = baseContext();
    auto syncopated = straight;
    straight.parameters.syncopation = 0;
    syncopated.parameters.syncopation = 100;

    MusicalEngine engine;

    const auto a = engine.generate(straight, 99);
    const auto b = engine.generate(syncopated, 99);

    std::set<double> timesA;
    std::set<double> timesB;

    for (const auto& n : a.notes)
        timesA.insert(n.startBeat);

    for (const auto& n : b.notes)
        timesB.insert(n.startBeat);

    expect(timesA != timesB, "syncopation changes onset pattern");
}

void testVariationChangesLaterMotif()
{
    auto stable = baseContext();
    auto varied = stable;
    stable.parameters.variation = 0;
    varied.parameters.variation = 100;
    stable.parameters.lengthBars = 8;
    varied.parameters.lengthBars = 8;
    stable.parameters.phraseLengthBars = 4;
    varied.parameters.phraseLengthBars = 4;

    MusicalEngine engine;

    const auto a = engine.generate(stable, 17);
    const auto b = engine.generate(varied, 17);

    expect(a.notes.size() == b.notes.size(), "variation preserves rhythm density");

    bool pitchChanged = false;

    for (std::size_t i = 0; i < a.notes.size(); ++i)
    {
        if (a.notes[i].midiNote != b.notes[i].midiNote)
        {
            pitchChanged = true;
            break;
        }
    }

    expect(pitchChanged, "variation changes pitches");
}


void testRoleSpecificGeneration()
{
    MusicalEngine engine;
    auto base = baseContext();

    base.parameters.lengthBars = 4;
    base.parameters.phraseLengthBars = 4;
    base.parameters.density = 60;

    base.role = Role::Bass;
    const auto bass = engine.generate(base, 100);

    base.role = Role::Arp;
    const auto arp = engine.generate(base, 100);

    base.role = Role::Chords;
    const auto chords = engine.generate(base, 100);

    base.role = Role::Pad;
    const auto pad = engine.generate(base, 100);

    expect(!bass.notes.empty(), "bass produces notes");
    expect(!arp.notes.empty(), "arp produces notes");
    expect(!chords.notes.empty(), "chords produce notes");
    expect(!pad.notes.empty(), "pad produces notes");

    // Chords must actually be polyphonic in the model.
    bool hasSameStart = false;
    for (std::size_t i = 1; i < chords.notes.size(); ++i)
    {
        if (chords.notes[i].startBeat == chords.notes[i - 1].startBeat)
        {
            hasSameStart = true;
            break;
        }
    }
    expect(hasSameStart, "chord role is polyphonic");

    expect(arp.notes.size() >= bass.notes.size(),
           "arp is at least as dense as bass");

    for (const auto& note : pad.notes)
        expect(note.durationBeats >= 2.5, "pad notes are sustained");

    expect(bass.isValid(), "bass phrase validates");
    expect(arp.isValid(), "arp phrase validates");
    expect(chords.isValid(), "chord phrase validates");
    expect(pad.isValid(), "pad phrase validates");
}


void testConfiguredRangeUsesBothEnds()
{
    auto context = baseContext();
    context.parameters.octaveLow =
        AbletonOctaveConvention::abletonOctaveToInternal(0);
    context.parameters.octaveHigh =
        AbletonOctaveConvention::abletonOctaveToInternal(5);
    context.parameters.density = 100;
    context.parameters.variation = 50;

    MusicalEngine engine;
    const auto phrase = engine.generate(context, 321);

    expect(
        !phrase.notes.empty(),
        "wide-range phrase contains notes");

    const int lowMidi =
        12 * (context.parameters.octaveLow + 1);

    const int highMidi =
        12 * (context.parameters.octaveHigh + 1);

    const auto allowed =
        context.scale.getPitchClasses(
            toPitchClass(context.key));

    int expectedLowest = 127;
    int expectedHighest = 0;

    for (int midi = lowMidi;
         midi <= highMidi;
         ++midi)
    {
        if (std::find(
                allowed.begin(),
                allowed.end(),
                midi % 12) != allowed.end())
        {
            expectedLowest =
                std::min(
                    expectedLowest,
                    midi);

            expectedHighest =
                std::max(
                    expectedHighest,
                    midi);
        }
    }

    int actualLowest = 127;
    int actualHighest = 0;

    for (const auto& note : phrase.notes)
    {
        actualLowest =
            std::min(
                actualLowest,
                note.midiNote);

        actualHighest =
            std::max(
                actualHighest,
                note.midiNote);
    }

    expect(
        actualLowest == expectedLowest,
        "generation reaches lowest valid range note");

    expect(
        actualHighest == expectedHighest,
        "generation reaches highest valid range note");

    expect(
        expectedHighest == highMidi,
        "highest configured C octave is the absolute upper boundary");
}


void testConfiguredAbletonOctaveRange()
{
    auto context = baseContext();

    context.parameters.octaveLow =
        AbletonOctaveConvention::
            abletonOctaveToInternal(2);

    context.parameters.octaveHigh =
        AbletonOctaveConvention::
            abletonOctaveToInternal(4);

    context.parameters.density = 100;

    MusicalEngine engine;
    const auto phrase =
        engine.generate(
            context,
            5522);

    expect(
        !phrase.notes.empty(),
        "Ableton octave range generates notes");

    const int expectedLow =
        AbletonOctaveConvention::
            midiForC(2);

    const int expectedHigh =
        AbletonOctaveConvention::
            midiForC(4);

    int actualLow = 127;
    int actualHigh = 0;

    for (const auto& note : phrase.notes)
    {
        actualLow =
            std::min(
                actualLow,
                note.midiNote);

        actualHigh =
            std::max(
                actualHigh,
                note.midiNote);

        expect(
            note.midiNote >= expectedLow &&
            note.midiNote <= expectedHigh,
            "generated notes stay inside Ableton-labelled C2-C4 range");
    }

    expect(
        actualLow == expectedLow,
        "configured Ableton low C is reachable");

    expect(
        actualHigh == expectedHigh,
        "configured Ableton high C is reachable");
}

void testOctaveShiftStaysInsideAbsoluteRange()
{
    auto context = baseContext();
    context.parameters.octaveLow =
        AbletonOctaveConvention::abletonOctaveToInternal(0);
    context.parameters.octaveHigh =
        AbletonOctaveConvention::abletonOctaveToInternal(5);
    context.parameters.octaveShift = 2;
    context.parameters.density = 100;

    MusicalEngine engine;
    const auto phrase =
        engine.generate(context, 654);

    const int lowMidi =
        12 * (context.parameters.octaveLow + 1);

    const int highMidi =
        12 * (context.parameters.octaveHigh + 1);

    for (const auto& note : phrase.notes)
    {
        expect(
            note.midiNote >= lowMidi &&
            note.midiNote <= highMidi,
            "octave shift stays inside absolute range");
    }
}


void testRepetitionControlChangesPhraseReuse()
{
    auto context = baseContext();
    context.parameters.lengthBars = 8;
    context.parameters.phraseLengthBars = 4;
    context.parameters.variation = 70;

    MusicalEngine engine;

    context.parameters.repetition = 100;
    const auto repeated =
        engine.generate(context, 9001);

    context.parameters.repetition = 0;
    const auto varied =
        engine.generate(context, 9001);

    expect(
        repeated.notes.size() == varied.notes.size(),
        "repetition preserves phrase topology");

    bool differs = false;

    for (std::size_t i = 0;
         i < repeated.notes.size() &&
         i < varied.notes.size();
         ++i)
    {
        if (repeated.notes[i].midiNote !=
            varied.notes[i].midiNote)
        {
            differs = true;
            break;
        }
    }

    expect(
        differs,
        "repetition changes later phrase pitch reuse");
}

void testComplexityAffectsPitchChoices()
{
    auto context = baseContext();
    context.parameters.lengthBars = 4;
    context.parameters.phraseLengthBars = 4;
    context.parameters.variation = 40;
    context.parameters.tension = 40;

    MusicalEngine engine;

    context.parameters.complexity = 0;
    const auto simple =
        engine.generate(context, 7331);

    context.parameters.complexity = 100;
    const auto complex =
        engine.generate(context, 7331);

    expect(
        simple.notes.size() == complex.notes.size(),
        "complexity preserves phrase topology");

    bool differs = false;

    for (std::size_t i = 0;
         i < simple.notes.size() &&
         i < complex.notes.size();
         ++i)
    {
        if (simple.notes[i].midiNote !=
            complex.notes[i].midiNote)
        {
            differs = true;
            break;
        }
    }

    expect(
        differs,
        "complexity changes pitch choices");
}



void testExplicitNoteLengthAffectsDuration()
{
    auto context = baseContext();
    context.parameters.lengthBars = 4;
    context.parameters.phraseLengthBars = 4;
    context.parameters.density = 45;

    MusicalEngine engine;

    context.parameters.noteLength =
        midigengx::domain::NoteLength::Short;

    const auto shortPhrase =
        engine.generate(context, 4567);

    context.parameters.noteLength =
        midigengx::domain::NoteLength::Long;

    const auto longPhrase =
        engine.generate(context, 4567);

    expect(
        shortPhrase.notes.size() == longPhrase.notes.size(),
        "note length preserves phrase topology");

    bool durationDiffers = false;

    for (std::size_t i = 0;
         i < shortPhrase.notes.size() &&
         i < longPhrase.notes.size();
         ++i)
    {
        if (shortPhrase.notes[i].durationBeats !=
            longPhrase.notes[i].durationBeats)
        {
            durationDiffers = true;
            break;
        }
    }

    expect(
        durationDiffers,
        "explicit Note Length changes durations");
}

void testHumanizationChangesTimingDeterministically()
{
    auto context = baseContext();
    context.parameters.lengthBars = 4;
    context.parameters.phraseLengthBars = 4;
    context.parameters.density = 50;

    MusicalEngine engine;

    context.parameters.humanization = 0;
    const auto dry =
        engine.generate(context, 9876);

    context.parameters.humanization = 100;
    const auto wet =
        engine.generate(context, 9876);

    expect(
        dry.notes.size() == wet.notes.size(),
        "humanization preserves phrase topology");

    bool timingDiffers = false;

    for (std::size_t i = 0;
         i < dry.notes.size() &&
         i < wet.notes.size();
         ++i)
    {
        if (dry.notes[i].startBeat !=
            wet.notes[i].startBeat ||
            dry.notes[i].velocity !=
            wet.notes[i].velocity)
        {
            timingDiffers = true;
            break;
        }
    }

    expect(
        timingDiffers,
        "humanization changes timing or velocity");
}



void testCadenceRootEndsOnRootPitchClass()
{
    auto context = baseContext();
    context.parameters.lengthBars = 4;
    context.parameters.phraseLengthBars = 4;
    context.parameters.density = 100;
    context.parameters.cadenceStyle =
        midigengx::domain::CadenceStyle::Root;
    context.parameters.cadenceStrength = 100;

    MusicalEngine engine;
    const auto phrase =
        engine.generate(context, 4242);

    expect(
        !phrase.notes.empty(),
        "cadence test generated notes");

    const int rootPitchClass =
        toPitchClass(context.key);

    const int lastPitchClass =
        ((phrase.notes.back().midiNote % 12) + 12) % 12;

    expect(
        lastPitchClass == rootPitchClass,
        "root cadence resolves final note to tonic");
}

void testContourChangesRegisterPath()
{
    auto context = baseContext();
    context.parameters.lengthBars = 8;
    context.parameters.phraseLengthBars = 8;
    context.parameters.density = 100;
    context.parameters.cadenceStrength = 0;
    context.parameters.octaveLow = 1;
    context.parameters.octaveHigh = 6;

    MusicalEngine engine;

    context.parameters.phraseContour =
        midigengx::domain::PhraseContour::Ascending;

    const auto ascending =
        engine.generate(context, 8181);

    context.parameters.phraseContour =
        midigengx::domain::PhraseContour::Descending;

    const auto descending =
        engine.generate(context, 8181);

    expect(
        ascending.notes.size() ==
            descending.notes.size(),
        "contour preserves phrase topology");

    expect(
        !ascending.notes.empty() &&
        !descending.notes.empty(),
        "contour test generated notes");

    if (!ascending.notes.empty() &&
        !descending.notes.empty())
    {
        expect(
            ascending.notes.front().midiNote <
                ascending.notes.back().midiNote,
            "ascending contour rises from first to last note");

        expect(
            descending.notes.front().midiNote >
                descending.notes.back().midiNote,
            "descending contour falls from first to last note");

        expect(
            ascending.notes.front().midiNote !=
                descending.notes.front().midiNote,
            "contours use different starting registers");
    }
}



void testPhraseAnalysis()
{
    Phrase phrase;

    phrase.lengthBeats = 4.0;
    phrase.notes =
    {
        NoteEvent{60, 100, 0.0, 1.0, 1},
        NoteEvent{64, 96, 1.0, 0.5, 1},
        NoteEvent{67, 92, 2.0, 1.0, 1}
    };

    const auto analysis =
        analyzePhrase(phrase);

    expect(
        analysis.noteCount == 3,
        "phrase analysis counts notes");

    expect(
        analysis.lowestMidi == 60 &&
            analysis.highestMidi == 67,
        "phrase analysis tracks pitch range");

    expect(
        analysis.pitchClasses.size() == 3,
        "phrase analysis extracts pitch classes");

    expect(
        analysis.intervalSemitones.size() == 2 &&
            analysis.intervalSemitones[0] == 4 &&
            analysis.intervalSemitones[1] == 3,
        "phrase analysis extracts melodic intervals");

    expect(
        analysis.isMonophonic,
        "phrase analysis detects monophonic phrase");

    expect(
        isPhraseContainedInRange(
            phrase,
            60,
            67),
        "phrase range validator accepts contained phrase");

    expect(
        isPhraseScaleContained(
            phrase,
            {0, 2, 4, 5, 7, 9, 11}),
        "phrase scale validator accepts contained pitch classes");
}

void testGenerationIntentNormalization()
{
    midigengx::domain::GenerationIntent intent;

    intent.lyrical = -20;
    intent.energetic = 160;
    intent.dark = 45;
    intent.context.parameters.density = 140;

    intent.normalize();

    expect(
        intent.lyrical == 0,
        "generation intent clamps negative semantic values");

    expect(
        intent.energetic == 100,
        "generation intent clamps high semantic values");

    expect(
        intent.dark == 45,
        "generation intent preserves valid semantic values");

    expect(
        intent.context.parameters.density == 100,
        "generation intent normalizes musical context");
}


void testMotifRoundTrip()
{
    Phrase phrase;
    phrase.lengthBeats = 4.0;
    phrase.notes =
    {
        NoteEvent{64, 108, 0.0, 0.5, 1},
        NoteEvent{67, 100, 1.0, 0.5, 1},
        NoteEvent{60, 94, 2.0, 1.0, 1}
    };

    const auto motif =
        extractMotif(
            phrase,
            4.0);

    expect(
        motif.isValid(),
        "motif extraction creates a valid motif");

    expect(
        motif.notes.size() == 3,
        "motif preserves source note count");

    expect(
        motif.notes[0].relativePitch == 0 &&
            motif.notes[1].relativePitch == 3 &&
            motif.notes[2].relativePitch == -4,
        "motif stores transposition-independent intervals");

    const auto applied =
        applyMotif(
            motif,
            72,
            4.0);

    expect(
        applied.isValid(),
        "motif application creates a valid phrase");

    expect(
        applied.notes[0].midiNote == 72 &&
            applied.notes[1].midiNote == 75 &&
            applied.notes[2].midiNote == 68,
        "motif applies relative pitch from a new origin");

    expect(
        applied.notes[0].startBeat == 4.0 &&
            applied.notes[1].startBeat == 5.0,
        "motif preserves relative timing");

    expect(
        applied.lengthBeats == 7.0,
        "motif application preserves phrase extent");
}


void testMotifDevelopment()
{
    Motif motif;
    motif.lengthBeats = 2.0;
    motif.notes =
    {
        MotifNote{0.0, 0.5, 0, 0},
        MotifNote{0.5, 0.5, 3, 4},
        MotifNote{1.0, 1.0, 5, -2}
    };

    expect(
        motif.isValid(),
        "development test starts with valid motif");

    const auto transposed =
        MotifDevelopment::transpose(
            motif,
            7);

    expect(
        transposed.notes[0].relativePitch == 7 &&
            transposed.notes[1].relativePitch == 10 &&
            transposed.notes[2].relativePitch == 12,
        "motif transpose shifts every interval consistently");

    const auto inverted =
        MotifDevelopment::invert(motif);

    expect(
        inverted.notes[0].relativePitch == 0 &&
            inverted.notes[1].relativePitch == -3 &&
            inverted.notes[2].relativePitch == -5,
        "motif inversion mirrors intervals around the origin");

    const auto stretched =
        MotifDevelopment::stretchTime(
            motif,
            2.0);

    expect(
        stretched.lengthBeats == 4.0 &&
            stretched.notes[1].startBeat == 1.0 &&
            stretched.notes[2].durationBeats == 2.0,
        "motif time stretch scales timing and duration");

    const auto repeated =
        MotifDevelopment::repeat(
            motif,
            3,
            0.5);

    expect(
        repeated.notes.size() == 9,
        "motif repetition preserves note count per repetition");

    expect(
        repeated.notes[3].startBeat == 2.5,
        "motif repetition applies deterministic period offset");

    expect(
        repeated.lengthBeats == 7.0,
        "motif repetition computes total developed length");

    const auto combined =
        MotifDevelopment::transposeAndStretch(
            motif,
            4,
            0.5);

    expect(
        combined.isValid() &&
            combined.lengthBeats == 1.0 &&
            combined.notes[1].relativePitch == 7,
        "combined motif development composes transformations");
}


void testMotifPhraseComposer()
{
    auto context = baseContext();

    context.parameters.lengthBars = 8;
    context.parameters.phraseLengthBars = 4;
    context.parameters.octaveLow = 1;
    context.parameters.octaveHigh = 4;
    context.parameters.variation = 65;
    context.parameters.repetition = 25;

    Motif seedMotif;
    seedMotif.lengthBeats = 2.0;
    seedMotif.notes =
    {
        MotifNote{0.0, 0.5, 0, 0},
        MotifNote{0.5, 0.5, 2, 3},
        MotifNote{1.0, 0.5, 4, -2},
        MotifNote{1.5, 0.5, 2, 1}
    };

    const auto first =
        MotifPhraseComposer::compose(
            seedMotif,
            context,
            12001);

    const auto second =
        MotifPhraseComposer::compose(
            seedMotif,
            context,
            12001);

    expect(
        first.isValid(),
        "motif phrase composer produces a valid phrase");

    expect(
        first.lengthBeats == 32.0,
        "motif phrase composer respects total phrase length");

    expect(
        first.notes.size() == second.notes.size(),
        "motif phrase composer is deterministic");

    bool identical = true;

    for (std::size_t i = 0; i < first.notes.size(); ++i)
    {
        if (first.notes[i].midiNote != second.notes[i].midiNote ||
            first.notes[i].startBeat != second.notes[i].startBeat ||
            first.notes[i].durationBeats != second.notes[i].durationBeats)
        {
            identical = false;
            break;
        }
    }

    expect(
        identical,
        "motif phrase composer repeats deterministically with same seed");

    const auto allowed =
        context.scale.getPitchClasses(
            toPitchClass(context.key));

    const int lowMidi =
        12 * (context.parameters.octaveLow + 1);

    const int highMidi =
        12 * (context.parameters.octaveHigh + 1);

    for (const auto& note : first.notes)
    {
        expect(
            note.midiNote >= lowMidi &&
            note.midiNote <= highMidi,
            "motif phrase composer respects configured register");

        expect(
            std::find(
                allowed.begin(),
                allowed.end(),
                note.midiNote % 12) != allowed.end(),
            "motif phrase composer respects selected scale");
    }

    auto exactRepeat = context;
    exactRepeat.parameters.repetition = 100;
    exactRepeat.parameters.variation = 65;

    auto developed = context;
    developed.parameters.repetition = 0;
    developed.parameters.variation = 65;

    const auto repeated =
        MotifPhraseComposer::compose(
            seedMotif,
            exactRepeat,
            44001);

    const auto varied =
        MotifPhraseComposer::compose(
            seedMotif,
            developed,
            44001);

    expect(
        repeated.notes.size() == varied.notes.size(),
        "motif development preserves phrase note count");

    bool changed = false;

    for (std::size_t i = 0;
         i < repeated.notes.size();
         ++i)
    {
        if (repeated.notes[i].midiNote !=
                varied.notes[i].midiNote ||
            repeated.notes[i].startBeat !=
                varied.notes[i].startBeat ||
            repeated.notes[i].durationBeats !=
                varied.notes[i].durationBeats)
        {
            changed = true;
            break;
        }
    }

    expect(
        changed,
        "variation/repetition controls deterministically affect motif development");
}


void testPhraseStructureAndCadence()
{
    auto context = baseContext();
    context.parameters.lengthBars = 12;
    context.parameters.phraseLengthBars = 4;
    context.parameters.tension = 65;
    context.parameters.cadenceStrength = 100;
    context.parameters.cadenceStyle = CadenceStyle::Root;

    const auto plan =
        planPhraseStructure(context);

    expect(
        plan.isValid(),
        "phrase structure plan is valid");

    expect(
        plan.sections.size() == 3,
        "phrase structure creates opening, preparation/development and cadence sections");

    expect(
        plan.sections.front().section == PhraseSection::Opening,
        "phrase structure starts with opening");

    expect(
        plan.sections.back().section == PhraseSection::Cadence,
        "phrase structure ends with cadence");

    expect(
        plan.sections.back().targetScaleDegree == 0,
        "root cadence targets scale degree zero");

    auto fifth = context;
    fifth.parameters.cadenceStyle = CadenceStyle::Fifth;

    const auto fifthPlan =
        planPhraseStructure(fifth);

    expect(
        fifthPlan.sections.back().targetScaleDegree == 4,
        "fifth cadence targets scale degree four");

    expect(
        cadenceTargetScaleDegree(
            CadenceStyle::Third,
            7) == 2,
        "third cadence maps to third scale degree");
}


void testHarmonyPlanning()
{
    auto context = baseContext();
    context.parameters.lengthBars = 12;
    context.parameters.phraseLengthBars = 4;
    context.parameters.cadenceStyle = CadenceStyle::Root;
    context.parameters.cadenceStrength = 100;

    const auto structure =
        planPhraseStructure(context);

    const auto harmony =
        planHarmony(
            context,
            structure);

    expect(
        harmony.isValid(),
        "harmony plan is valid");

    expect(
        harmony.events.size() ==
            structure.sections.size(),
        "harmony plan mirrors phrase sections");

    expect(
        harmony.events.front().scaleDegree == 0,
        "harmony opens on tonic");

    expect(
        harmony.events.back().scaleDegree == 0,
        "root cadence resolves to tonic");

    const auto rootQuality =
        inferTriadQuality(
            context.scale.getPitchClasses(
                toPitchClass(context.key)),
            0);

    expect(
        rootQuality == ChordQuality::Minor,
        "C minor natural scale has a minor tonic triad");

    auto majorContext = context;
    majorContext.scale = Scale{ScaleType::Major};

    const auto majorStructure =
        planPhraseStructure(
            majorContext);

    const auto majorHarmony =
        planHarmony(
            majorContext,
            majorStructure);

    expect(
        majorHarmony.events.front().quality ==
            ChordQuality::Major,
        "major scale tonic triad is major");

    auto fifthContext = context;
    fifthContext.parameters.cadenceStyle =
        CadenceStyle::Fifth;

    const auto fifthStructure =
        planPhraseStructure(
            fifthContext);

    const auto fifthHarmony =
        planHarmony(
            fifthContext,
            fifthStructure);

    expect(
        fifthHarmony.events.back().scaleDegree == 4,
        "fifth cadence changes final harmonic target");
}


void testHarmonyAwareMotifComposition()
{
    auto context = baseContext();
    context.parameters.lengthBars = 8;
    context.parameters.phraseLengthBars = 4;
    context.parameters.octaveLow = 1;
    context.parameters.octaveHigh = 4;
    context.parameters.cadenceStrength = 100;
    context.parameters.repetition = 100;
    context.parameters.variation = 0;

    Motif seed;
    seed.lengthBeats = 4.0;
    seed.notes =
    {
        MotifNote{0.0, 1.0, 0, 0},
        MotifNote{1.0, 1.0, 2, 0},
        MotifNote{2.0, 1.0, 4, 0},
        MotifNote{3.0, 1.0, 2, 0}
    };

    const auto phrase =
        MotifPhraseComposer::compose(
            seed,
            context,
            20208);

    const auto structure =
        planPhraseStructure(context);

    const auto harmony =
        planHarmony(context, structure);

    expect(
        harmony.isValid(),
        "harmony-aware composition has valid harmony plan");

    expect(
        !phrase.notes.empty(),
        "harmony-aware composer generates notes");

    for (const auto& note : phrase.notes)
    {
        const auto* event =
            findHarmonyEventAtBeat(
                harmony,
                note.startBeat);

        expect(
            event != nullptr,
            "every composed note belongs to a harmony event");

        const auto guidance =
            buildHarmonyGuidance(
                harmony,
                *event,
                context.scale.getPitchClasses(
                    toPitchClass(context.key)),
                100);

        const double beatInSection =
            note.startBeat - event->startBeat;

        const bool onStrongBeat =
            std::fmod(
                std::max(0.0, beatInSection),
                4.0) < 1.0e-9;

        if (onStrongBeat)
        {
            expect(
                guidance.isChordTone(
                    note.midiNote % 12),
                "strong-beat motif notes resolve to chord tones");
        }
    }
}


void testRhythmPlanningAndDevelopment()
{
    auto context = baseContext();
    context.parameters.density = 100;
    context.parameters.syncopation = 0;
    context.parameters.noteLength =
        NoteLength::Short;
    context.parameters.rhythm =
        RhythmFeel::Straight;

    Motif motif;
    motif.lengthBeats = 4.0;
    motif.notes =
    {
        MotifNote{0.0, 1.0, 0, 0},
        MotifNote{0.7, 1.0, 2, 0},
        MotifNote{1.8, 1.0, 4, 0},
        MotifNote{2.6, 1.0, 5, 0}
    };

    const auto plan =
        planRhythm(context);

    expect(
        plan.isValid(),
        "rhythm plan is valid");

    const auto straight =
        applyRhythmPlan(
            motif,
            plan);

    expect(
        straight.isValid(),
        "straight rhythm development is valid");

    expect(
        straight.notes.size() == motif.notes.size(),
        "full density keeps motif events");

    expect(
        straight.notes.front().startBeat == 0.0,
        "straight rhythm quantizes first onset");

    for (const auto& note : straight.notes)
    {
        expect(
            note.durationBeats == 0.25,
            "short note length produces short duration");
    }

    auto syncopatedContext = context;
    syncopatedContext.parameters.syncopation = 100;
    syncopatedContext.parameters.rhythm =
        RhythmFeel::Offbeat;

    const auto syncPlan =
        planRhythm(syncopatedContext);

    const auto syncopated =
        applyRhythmPlan(
            motif,
            syncPlan);

    expect(
        syncopated.isValid(),
        "syncopated rhythm development is valid");

    bool onsetChanged = false;

    for (std::size_t i = 0;
         i < straight.notes.size() &&
         i < syncopated.notes.size();
         ++i)
    {
        if (straight.notes[i].startBeat !=
            syncopated.notes[i].startBeat)
        {
            onsetChanged = true;
            break;
        }
    }

    expect(
        onsetChanged,
        "syncopation changes onset placement");

    auto sparseContext = context;
    sparseContext.parameters.density = 10;

    const auto sparse =
        applyRhythmPlan(
            motif,
            planRhythm(sparseContext));

    expect(
        !sparse.notes.empty(),
        "low density still preserves a valid motif event");

    expect(
        sparse.notes.size() < motif.notes.size(),
        "density reduces rhythmic event count");

    auto longContext = context;
    longContext.parameters.noteLength =
        NoteLength::Long;

    const auto longRhythm =
        applyRhythmPlan(
            motif,
            planRhythm(longContext));

    expect(
        longRhythm.notes.front().durationBeats >
            straight.notes.front().durationBeats,
        "long note length increases duration");

    auto densityOnlyContext = context;
    densityOnlyContext.parameters.density = 100;
    densityOnlyContext.parameters.noteLengthVariation = 0;

    const auto densityOnly =
        applyRhythmPlan(
            motif,
            planRhythm(densityOnlyContext));

    expect(
        densityOnly.notes.back().durationBeats ==
            densityOnly.notes.front().durationBeats,
        "density does not alter note duration");

    auto lengthVariationContext = context;
    lengthVariationContext.parameters.noteLengthVariation = 50;

    const auto variedLengths =
        applyRhythmPlan(
            motif,
            planRhythm(lengthVariationContext));

    expect(
        variedLengths.notes.back().durationBeats <
            variedLengths.notes.front().durationBeats,
        "note length variation alters selected durations");
}


void testMelodicContourPolicy()
{
    using Contour = midigengx::domain::PhraseContour;

    expect(
        MelodicContour::registerShape(
            Contour::Ascending,
            0.0) == 0.0,
        "ascending contour starts low");

    expect(
        MelodicContour::registerShape(
            Contour::Ascending,
            1.0) == 1.0,
        "ascending contour ends high");

    expect(
        MelodicContour::registerShape(
            Contour::Descending,
            0.0) == 1.0,
        "descending contour starts high");

    expect(
        MelodicContour::registerShape(
            Contour::Descending,
            1.0) == 0.0,
        "descending contour ends low");

    expect(
        MelodicContour::registerShape(
            Contour::Flat,
            0.0) == 0.5 &&
        MelodicContour::registerShape(
            Contour::Flat,
            1.0) == 0.5,
        "flat contour keeps centered register");

    const int low = 0;
    const int high = 100;

    expect(
        MelodicContour::endpointRegisterIndex(
            Contour::Ascending,
            0,
            8,
            low,
            high) == low,
        "ascending endpoint policy starts at low register");

    expect(
        MelodicContour::endpointRegisterIndex(
            Contour::Ascending,
            7,
            8,
            low,
            high) == high,
        "ascending endpoint policy ends at high register");

    expect(
        MelodicContour::endpointRegisterIndex(
            Contour::Descending,
            0,
            8,
            low,
            high) == high,
        "descending endpoint policy starts at high register");

    expect(
        MelodicContour::endpointRegisterIndex(
            Contour::Descending,
            7,
            8,
            low,
            high) == low,
        "descending endpoint policy ends at low register");

    expect(
        MelodicContour::endpointRegisterIndex(
            Contour::Arch,
            4,
            8,
            low,
            high) == high,
        "arch endpoint policy peaks in the middle");

    expect(
        MelodicContour::endpointRegisterIndex(
            Contour::Valley,
            4,
            8,
            low,
            high) == low,
        "valley endpoint policy reaches its low point in the middle");

    expect(
        MelodicContour::registerShape(
            Contour::Ascending,
            -1.0) == 0.0 &&
        MelodicContour::registerShape(
            Contour::Ascending,
            2.0) == 1.0,
        "contour register shape clamps progress");

    expect(
        MelodicContour::endpointRegisterIndex(
            Contour::Ascending,
            0,
            0,
            low,
            high) == low,
        "contour endpoint policy handles empty note count");
}


void testMelodicMotionModel()
{
    const auto unison =
        MelodicMotion::analyzeInterval(
            60,
            60);

    expect(
        unison.direction ==
            MotionDirection::Stationary &&
        unison.interval ==
            IntervalClass::Unison,
        "motion model detects unison");

    const auto upwardStep =
        MelodicMotion::analyzeInterval(
            60,
            62);

    expect(
        upwardStep.direction ==
            MotionDirection::Up &&
        upwardStep.interval ==
            IntervalClass::Step &&
        upwardStep.semitoneDistance == 2,
        "motion model detects upward step");

    const auto downwardLeap =
        MelodicMotion::analyzeInterval(
            67,
            60);

    expect(
        downwardLeap.direction ==
            MotionDirection::Down &&
        downwardLeap.interval ==
            IntervalClass::LargeLeap,
        "motion model detects downward leap");

    const auto octave =
        MelodicMotion::analyzeInterval(
            60,
            72);

    expect(
        octave.interval ==
            IntervalClass::Octave,
        "motion model identifies octave");

    const int simpleLimit =
        MelodicMotion::preferredMaximumLeap(
            0,
            0);

    const int complexLimit =
        MelodicMotion::preferredMaximumLeap(
            100,
            100);

    expect(
        simpleLimit <= complexLimit &&
        simpleLimit >= 2 &&
        complexLimit <= 7,
        "complexity and tension widen preferred leap envelope");

    expect(
        MelodicMotion::scoreInterval(
            2,
            0,
            0) >
        MelodicMotion::scoreInterval(
            9,
            0,
            0),
        "stepwise motion scores above large leaps at low complexity");

    expect(
        MelodicMotion::scoreInterval(
            7,
            100,
            100) >=
        MelodicMotion::scoreInterval(
            7,
            0,
            0),
        "complexity and tension permit larger melodic motion");
}


void testMelodicMotionComposerIntegration()
{
    auto context = baseContext();
    context.parameters.lengthBars = 4;
    context.parameters.phraseLengthBars = 4;
    context.parameters.density = 100;
    context.parameters.variation = 0;
    context.parameters.repetition = 100;
    context.parameters.complexity = 0;
    context.parameters.tension = 0;

    Motif seed;
    seed.lengthBeats = 4.0;
    seed.notes =
    {
        MotifNote{0.0, 0.5, 0, 0},
        MotifNote{1.0, 0.5, 7, 0},
        MotifNote{2.0, 0.5, 2, 0},
        MotifNote{3.0, 0.5, 8, 0}
    };

    const auto structure =
        planPhraseStructure(context);

    const auto harmony =
        planHarmony(context, structure);

    const auto scalePitchClasses =
        context.scale.getPitchClasses(
            toPitchClass(context.key));

    const auto guidance =
        buildHarmonyGuidance(
            harmony,
            harmony.events.front(),
            scalePitchClasses,
            100);

    const int guided =
        chooseMelodicMotionPitch(
            72,
            60,
            48,
            84,
            scalePitchClasses,
            guidance,
            0,
            0,
            0.0);

    expect(
        guided >= 48 && guided <= 84,
        "motion guidance respects register");

    expect(
        std::find(
            scalePitchClasses.begin(),
            scalePitchClasses.end(),
            guided % 12) != scalePitchClasses.end(),
        "motion guidance respects scale");

    const auto composed =
        MotifPhraseComposer::compose(
            seed,
            context,
            121212);

    expect(
        composed.isValid() &&
        !composed.notes.empty(),
        "motion-aware composer produces a valid phrase");

    for (std::size_t i = 1;
         i < composed.notes.size();
         ++i)
    {
        const auto motion =
            MelodicMotion::analyzeInterval(
                composed.notes[i - 1].midiNote,
                composed.notes[i].midiNote);

        expect(
            motion.semitoneDistance <= 12,
            "motion-aware phrase avoids compound leaps");
    }
}


void testMelodicResolutionModel()
{
    expect(
        MelodicResolution::isLeap(
            60,
            64),
        "resolution model identifies a melodic leap");

    expect(
        !MelodicResolution::isLeap(
            60,
            62),
        "resolution model identifies stepwise motion");

    expect(
        MelodicResolution::preferredResolutionDirection(
            60,
            67) == -1,
        "ascending leap prefers downward resolution");

    expect(
        MelodicResolution::preferredResolutionDirection(
            67,
            60) == 1,
        "descending leap prefers upward resolution");

    expect(
        MelodicResolution::scoreResolution(
            60,
            67,
            65,
            0,
            0) >
        MelodicResolution::scoreResolution(
            60,
            67,
            72,
            0,
            0),
        "opposite-direction step resolves a leap better");

    expect(
        MelodicResolution::scoreResolution(
            60,
            67,
            65,
            80,
            80) >=
        MelodicResolution::scoreResolution(
            60,
            67,
            64,
            0,
            0),
        "complexity and tension support controlled resolution choices");
}


void testMelodicTendencyModel()
{
    auto context = baseContext();
    context.parameters.tension = 90;
    context.parameters.cadenceStrength = 100;

    const auto scalePitchClasses =
        context.scale.getPitchClasses(
            toPitchClass(context.key));

    const auto structure =
        planPhraseStructure(context);

    const auto harmony =
        planHarmony(
            context,
            structure);

    const auto guidance =
        buildHarmonyGuidance(
            harmony,
            harmony.events.back(),
            scalePitchClasses,
            100);

    const int tonic =
        toPitchClass(context.key);

    const int leadingTone =
        (tonic + 11) % 12;

    const auto tendency =
        analyzeTendencyTone(
            leadingTone,
            tonic,
            scalePitchClasses,
            guidance,
            90,
            100);

    expect(
        tendency.isTendency(),
        "leading tone is detected as a tendency tone");

    expect(
        tendency.direction ==
            TendencyDirection::ResolveUp &&
        tendency.targetPitchClass == tonic,
        "leading tone resolves upward to tonic");

    auto minorContext = baseContext();
    minorContext.scale =
        midigengx::domain::Scale(
            midigengx::domain::ScaleType::Minor);

    const int minorTonic =
        toPitchClass(minorContext.key);

    const auto minorScalePitchClasses =
        minorContext.scale.getPitchClasses(
            minorTonic);

    const auto minorStructure =
        planPhraseStructure(minorContext);

    const auto minorHarmony =
        planHarmony(
            minorContext,
            minorStructure);

    const auto minorGuidance =
        buildHarmonyGuidance(
            minorHarmony,
            minorHarmony.events.back(),
            minorScalePitchClasses,
            100);

    const auto chromaticLeadingTone =
        analyzeTendencyTone(
            (minorTonic + 11) % 12,
            minorTonic,
            minorScalePitchClasses,
            minorGuidance,
            90,
            100);

    expect(
        chromaticLeadingTone.isTendency(),
        "chromatic leading tone is detected outside natural minor scale");

    expect(
        scoreTendencyResolution(
            tonic,
            tendency,
            true) >
        scoreTendencyResolution(
            leadingTone,
            tendency,
            true),
        "cadential tendency target scores above unresolved tendency");

    const auto stable =
        analyzeTendencyTone(
            tonic,
            tonic,
            scalePitchClasses,
            guidance,
            10,
            10);

    expect(
        !stable.isTendency(),
        "tonic chord tone is stable");
}


void testPhraseTensionArc()
{
    using Contour = midigengx::domain::PhraseContour;

    expect(
        PhraseTensionArc::normalizedProgress(
            -1.0,
            16.0) == 0.0,
        "tension progress clamps below zero");

    expect(
        PhraseTensionArc::normalizedProgress(
            32.0,
            16.0) == 1.0,
        "tension progress clamps above one");

    expect(
        PhraseTensionArc::climaxPosition(
            Contour::Arch) == 0.5,
        "arch contour peaks in the middle");

    expect(
        PhraseTensionArc::climaxPosition(
            Contour::Ascending) == 1.0,
        "ascending contour peaks at the end");

    expect(
        PhraseTensionArc::climaxPosition(
            Contour::Descending) == 0.0,
        "descending contour peaks at the beginning");

    const int opening =
        PhraseTensionArc::tensionAtProgress(
            0.0,
            Contour::Arch,
            30,
            80);

    const int middle =
        PhraseTensionArc::tensionAtProgress(
            0.5,
            Contour::Arch,
            30,
            80);

    const int ending =
        PhraseTensionArc::tensionAtProgress(
            1.0,
            Contour::Arch,
            30,
            80);

    expect(
        middle > opening,
        "arch tension rises toward its climax");

    expect(
        ending >= opening,
        "cadence approach does not fall below opening tension");

    auto context = baseContext();
    context.parameters.lengthBars = 12;
    context.parameters.phraseLengthBars = 4;
    context.parameters.tension = 30;
    context.parameters.cadenceStrength = 100;
    context.parameters.phraseContour = Contour::Arch;

    const auto structure =
        planPhraseStructure(context);

    expect(
        structure.isValid(),
        "phrase tension arc preserves valid structure");

    expect(
        structure.sections.back().tension >=
            structure.sections.front().tension,
        "cadence section carries at least opening tension");
}


void testPhraseDevelopmentPlan()
{
    auto context = baseContext();
    context.parameters.lengthBars = 16;
    context.parameters.phraseLengthBars = 4;
    context.parameters.variation = 0;
    context.parameters.repetition = 100;
    context.parameters.tension = 40;

    const auto structure =
        planPhraseStructure(context);

    const auto repeated =
        planPhraseDevelopment(
            context,
            structure);

    expect(
        repeated.isValid(),
        "repetition development plan is valid");

    expect(
        repeated.sections.size() ==
            structure.sections.size(),
        "development plan mirrors phrase structure");

    expect(
        repeated.sections.front().role ==
            PhraseDevelopmentRole::Statement,
        "first phrase is a statement");

    bool hasRestatement = false;

    for (const auto& section : repeated.sections)
    {
        if (section.role ==
            PhraseDevelopmentRole::Restatement)
        {
            hasRestatement = true;
            expect(
                section.transpositionSemitones == 0 &&
                section.timeFactor == 1.0,
                "restatement preserves motif identity");
        }
    }

    expect(
        hasRestatement,
        "high repetition creates restatement sections");

    expect(
        repeated.sections[1].role ==
            PhraseDevelopmentRole::Restatement,
        "high repetition gives restatement priority over response");

    expect(
        repeated.sections[2].role ==
            PhraseDevelopmentRole::Intensification,
        "preparation remains intensification after restatement");

    auto varied = context;
    varied.parameters.repetition = 0;
    varied.parameters.variation = 100;

    const auto variedPlan =
        planPhraseDevelopment(
            varied,
            structure);

    bool hasDevelopment = false;
    bool hasResponseOrIntensity = false;

    for (const auto& section : variedPlan.sections)
    {
        if (section.role ==
            PhraseDevelopmentRole::Development)
            hasDevelopment = true;

        if (section.role ==
                PhraseDevelopmentRole::Response ||
            section.role ==
                PhraseDevelopmentRole::Intensification)
        {
            hasResponseOrIntensity = true;
        }
    }

    expect(
        hasDevelopment,
        "low repetition creates development sections");

    expect(
        hasResponseOrIntensity,
        "later phrase sections become response or intensification");

    auto shortForm = context;
    shortForm.parameters.lengthBars = 8;
    shortForm.parameters.phraseLengthBars = 4;
    shortForm.parameters.repetition = 100;
    shortForm.parameters.variation = 65;

    const auto shortStructure =
        planPhraseStructure(shortForm);

    const auto shortRepeat =
        planPhraseDevelopment(
            shortForm,
            shortStructure);

    shortForm.parameters.repetition = 0;

    const auto shortDevelop =
        planPhraseDevelopment(
            shortForm,
            shortStructure);

    expect(
        shortRepeat.sections.size() ==
            shortDevelop.sections.size(),
        "short-form development plans preserve topology");

    expect(
        shortRepeat.sections.back().transpositionSemitones !=
            shortDevelop.sections.back().transpositionSemitones,
        "variation/repetition affect the short-form cadence development");

    MotifPhraseComposer::compose(
        Motif{},
        context,
        1234);
}


void testMotifSequenceDevelopment()
{
    Motif motif;
    motif.lengthBeats = 2.0;
    motif.notes =
    {
        MotifNote{0.0, 0.5, 0, 0},
        MotifNote{1.0, 0.5, 2, 0}
    };

    const auto sequence =
        MotifDevelopment::sequence(
            motif,
            3,
            2);

    expect(
        sequence.isValid(),
        "motif sequence is valid");

    expect(
        sequence.notes.size() ==
            motif.notes.size() * 3,
        "sequence repeats the motif deterministically");

    expect(
        sequence.lengthBeats == 6.0,
        "sequence length equals repeated motif length");

    expect(
        sequence.notes[0].relativePitch == 0 &&
        sequence.notes[2].relativePitch == 2 &&
        sequence.notes[4].relativePitch == 4,
        "sequence applies incremental transposition");

    expect(
        sequence.notes[0].startBeat == 0.0 &&
        sequence.notes[2].startBeat == 2.0 &&
        sequence.notes[4].startBeat == 4.0,
        "sequence preserves ordered phrase timing");

    auto context = baseContext();
    context.parameters.lengthBars = 12;
    context.parameters.phraseLengthBars = 4;
    context.parameters.repetition = 0;
    context.parameters.variation = 100;

    const auto structure =
        planPhraseStructure(context);

    const auto development =
        planPhraseDevelopment(
            context,
            structure);

    bool foundSequence = false;

    for (const auto& section : development.sections)
    {
        if (section.sequenceRepetitions > 1)
        {
            foundSequence = true;

            expect(
                std::abs(
                    section.sequenceStepSemitones) > 0,
                "sequence section has a deterministic step");

            expect(
                section.sequenceRepetitions >= 2 &&
                section.sequenceRepetitions <= 8,
                "sequence repetitions stay bounded");
        }
    }

    expect(
        foundSequence,
        "high variation produces a phrase sequence section");
}


void testMotifResponseDevelopment()
{
    Motif motif;
    motif.lengthBeats = 4.0;
    motif.notes =
    {
        MotifNote{0.0, 0.5, 0, 0},
        MotifNote{1.0, 0.5, 4, 0},
        MotifNote{2.0, 1.0, 2, 0}
    };

    const auto response =
        MotifDevelopment::retrograde(motif);

    expect(
        response.isValid(),
        "retrograde response is valid");

    expect(
        response.notes.size() ==
            motif.notes.size(),
        "retrograde preserves note count");

    expect(
        response.notes.front().startBeat <
            response.notes.back().startBeat,
        "retrograde preserves chronological ordering");

    expect(
        response.notes.front().relativePitch ==
            motif.notes.back().relativePitch,
        "retrograde begins with original final pitch");

    auto context = baseContext();
    context.parameters.lengthBars = 20;
    context.parameters.phraseLengthBars = 4;
    context.parameters.repetition = 0;
    context.parameters.variation = 100;

    const auto structure =
        planPhraseStructure(context);

    const auto plan =
        planPhraseDevelopment(
            context,
            structure);

    expect(
        structure.sections.size() == 5,
        "20-bar context produces five phrase sections");

    expect(
        plan.sections.size() == 5,
        "development plan mirrors five-section topology");

    expect(
        plan.sections[2].role ==
            PhraseDevelopmentRole::Response,
        "five-section form places response before preparation");

    bool foundResponse =
        false;

    for (const auto& section : plan.sections)
    {
        if (section.role ==
            PhraseDevelopmentRole::Response)
        {
            foundResponse = true;

            expect(
                section.retrogradeMotif,
                "strong response development enables retrograde");

            expect(
                section.transpositionSemitones < 0,
                "response moves in an answering direction");
        }
    }

    expect(
        foundResponse,
        "phrase structure produces a response section");
}


void testPhraseRoleGrammarMatrix()
{
    auto context = baseContext();
    context.parameters.phraseLengthBars = 4;
    context.parameters.variation = 0;

    const auto roleAt =
        [](const PhraseDevelopmentPlan& plan,
           std::size_t index)
        {
            return plan.sections.at(index).role;
        };

    context.parameters.lengthBars = 8;
    context.parameters.repetition = 100;

    auto twoSections =
        planPhraseDevelopment(
            context,
            planPhraseStructure(context));

    expect(
        twoSections.sections.size() == 2 &&
        roleAt(twoSections, 0) ==
            PhraseDevelopmentRole::Statement &&
        roleAt(twoSections, 1) ==
            PhraseDevelopmentRole::Cadence,
        "two-section grammar is statement-cadence");

    context.parameters.lengthBars = 12;
    context.parameters.repetition = 100;

    auto threeSections =
        planPhraseDevelopment(
            context,
            planPhraseStructure(context));

    expect(
        threeSections.sections.size() == 3 &&
        roleAt(threeSections, 0) ==
            PhraseDevelopmentRole::Statement &&
        roleAt(threeSections, 1) ==
            PhraseDevelopmentRole::Intensification &&
        roleAt(threeSections, 2) ==
            PhraseDevelopmentRole::Cadence,
        "three-section grammar preserves preparation");

    context.parameters.lengthBars = 16;
    context.parameters.repetition = 100;

    auto fourRepeated =
        planPhraseDevelopment(
            context,
            planPhraseStructure(context));

    expect(
        fourRepeated.sections.size() == 4 &&
        roleAt(fourRepeated, 0) ==
            PhraseDevelopmentRole::Statement &&
        roleAt(fourRepeated, 1) ==
            PhraseDevelopmentRole::Restatement &&
        roleAt(fourRepeated, 2) ==
            PhraseDevelopmentRole::Intensification &&
        roleAt(fourRepeated, 3) ==
            PhraseDevelopmentRole::Cadence,
        "four-section high-repetition grammar is stable");

    context.parameters.repetition = 0;

    auto fourDeveloping =
        planPhraseDevelopment(
            context,
            planPhraseStructure(context));

    expect(
        fourDeveloping.sections.size() == 4 &&
        roleAt(fourDeveloping, 0) ==
            PhraseDevelopmentRole::Statement &&
        roleAt(fourDeveloping, 1) ==
            PhraseDevelopmentRole::Development &&
        roleAt(fourDeveloping, 2) ==
            PhraseDevelopmentRole::Intensification &&
        roleAt(fourDeveloping, 3) ==
            PhraseDevelopmentRole::Cadence,
        "four-section low-repetition grammar creates development");

    context.parameters.lengthBars = 20;
    context.parameters.repetition = 0;

    auto fiveSections =
        planPhraseDevelopment(
            context,
            planPhraseStructure(context));

    expect(
        fiveSections.sections.size() == 5 &&
        roleAt(fiveSections, 0) ==
            PhraseDevelopmentRole::Statement &&
        roleAt(fiveSections, 1) ==
            PhraseDevelopmentRole::Development &&
        roleAt(fiveSections, 2) ==
            PhraseDevelopmentRole::Response &&
        roleAt(fiveSections, 3) ==
            PhraseDevelopmentRole::Intensification &&
        roleAt(fiveSections, 4) ==
            PhraseDevelopmentRole::Cadence,
        "five-section grammar introduces response");
}


void testMotifIdentityVariation()
{
    Motif motif;
    motif.lengthBeats = 4.0;
    motif.notes =
    {
        MotifNote{0.0, 0.5, 0, 0},
        MotifNote{1.0, 0.5, 3, 0},
        MotifNote{2.0, 0.5, 5, 0},
        MotifNote{3.0, 0.5, 2, 0}
    };

    const auto unchanged =
        MotifDevelopment::varyIntervals(
            motif,
            0);

    expect(
        unchanged.notes.size() ==
            motif.notes.size(),
        "zero motif variation preserves note count");

    expect(
        unchanged.notes[0].relativePitch ==
            motif.notes[0].relativePitch,
        "zero motif variation preserves anchor");

    const auto varied =
        MotifDevelopment::varyIntervals(
            motif,
            100);

    expect(
        varied.isValid(),
        "high motif variation remains valid");

    expect(
        varied.notes.size() ==
            motif.notes.size(),
        "motif variation preserves note count");

    expect(
        varied.notes.front().startBeat ==
            motif.notes.front().startBeat &&
        varied.notes.back().startBeat ==
            motif.notes.back().startBeat,
        "motif variation preserves timing");

    expect(
        varied.notes.front().relativePitch ==
            motif.notes.front().relativePitch,
        "motif variation preserves first-note identity");

    bool changed =
        false;

    for (std::size_t i = 1;
         i < motif.notes.size();
         ++i)
    {
        if (varied.notes[i].relativePitch !=
            motif.notes[i].relativePitch)
        {
            changed = true;

            const int sourceInterval =
                motif.notes[i].relativePitch -
                motif.notes[i - 1].relativePitch;

            const int variedInterval =
                varied.notes[i].relativePitch -
                varied.notes[i - 1].relativePitch;

            expect(
                (sourceInterval > 0 &&
                 variedInterval > 0) ||
                (sourceInterval < 0 &&
                 variedInterval < 0),
                "motif variation preserves interval direction");
        }
    }

    expect(
        changed,
        "high motif variation changes at least one interval");

    auto context = baseContext();
    context.parameters.lengthBars = 12;
    context.parameters.phraseLengthBars = 4;
    context.parameters.repetition = 0;
    context.parameters.variation = 100;

    const auto plan =
        planPhraseDevelopment(
            context,
            planPhraseStructure(context));

    bool foundVariation =
        false;

    for (const auto& section : plan.sections)
    {
        if (section.role ==
                PhraseDevelopmentRole::Development ||
            section.role ==
                PhraseDevelopmentRole::Response ||
            section.role ==
                PhraseDevelopmentRole::Intensification)
        {
            if (section.motifVariationAmount > 0)
                foundVariation = true;
        }
    }

    expect(
        foundVariation,
        "development roles receive explicit motif variation");
}

void testPhraseValidity()
{
    auto context = baseContext();
    context.parameters.lengthBars = 8;
    context.parameters.phraseLengthBars = 4;
    context.parameters.noteLength = NoteLength::Staccato;

    MusicalEngine engine;
    const auto phrase = engine.generate(context, 7);

    expect(!phrase.notes.empty(), "phrase contains notes");
    expect(phrase.isValid(), "phrase validates");
    expect(phrase.lengthBeats == 32.0, "8 bars = 32 beats");
}

} // namespace

int main()
{
    testDeterministicGeneration();
    testScaleSafetyAndRange();
    testDensityControlsNoteCount();
    testSyncopationChangesRhythmicPlacement();
    testVariationChangesLaterMotif();
    testRepetitionControlChangesPhraseReuse();
    testComplexityAffectsPitchChoices();
    testExplicitNoteLengthAffectsDuration();
    testHumanizationChangesTimingDeterministically();
    testCadenceRootEndsOnRootPitchClass();
    testContourChangesRegisterPath();
    testConfiguredRangeUsesBothEnds();
    testConfiguredAbletonOctaveRange();
    testOctaveShiftStaysInsideAbsoluteRange();
    testMotifRoundTrip();
    testMotifDevelopment();
    testMotifPhraseComposer();
    testPhraseStructureAndCadence();
    testHarmonyPlanning();
    testHarmonyAwareMotifComposition();
    testRhythmPlanningAndDevelopment();
    testMelodicContourPolicy();
    testMelodicMotionModel();
    testMelodicMotionComposerIntegration();
    testMelodicResolutionModel();
    testMelodicTendencyModel();
    testPhraseTensionArc();
    testPhraseDevelopmentPlan();
    testPhraseRoleGrammarMatrix();
    testMotifSequenceDevelopment();
    testMotifIdentityVariation();
    testMotifResponseDevelopment();
    testPhraseValidity();
    testPhraseAnalysis();
    testGenerationIntentNormalization();
    testRoleSpecificGeneration();

    std::cout << "MIDI-GenGX Musical Engine tests passed.\n";
    return 0;
}
