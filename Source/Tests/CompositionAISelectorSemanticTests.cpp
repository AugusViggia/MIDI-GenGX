#include "Music/MusicalEngine.h"

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <vector>

using namespace midigengx::music;
using namespace midigengx::domain;

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

MusicalContext buildContext()
{
    MusicalContext context;
    context.key = Key::C;
    context.scale = Scale{ScaleType::Minor};
    context.role = Role::Lead;
    context.style = Style::ProgressiveHouse;
    context.character = Character::Emotional;

    context.parameters.lengthBars = 8;
    context.parameters.phraseLengthBars = 4;
    context.parameters.octaveLow = 0;
    context.parameters.octaveHigh = 2;
    context.parameters.density = 50;
    context.parameters.catchiness = 50;
    context.parameters.syncopation = 50;
    context.parameters.octaveMovement = 50;
    context.parameters.variation = 50;
    context.parameters.repetition = 50;
    context.parameters.tension = 50;
    context.parameters.complexity = 50;
    context.parameters.humanization = 0;
    context.parameters.noteLengthVariation = 0;
    context.parameters.cadenceStrength = 75;
    context.normalize();
    return context;
}

CompositionAIGuidance buildGuidance()
{
    CompositionAIGuidance guidance;
    guidance.roleTarget = 0.0;
    guidance.tensionTarget = 0.0;
    guidance.tensionDeltaTarget = 0.0;
    guidance.harmonyDegreeTarget = 0.0;
    guidance.harmonyQualityTarget = 0.0;
    guidance.harmonicDegreeDeltaTarget = 0.0;
    guidance.confidence = 1.0;

    // Neutral defaults: 50% for selector-style controls and 0% for
    // performance variation controls whose normalized neutral is represented
    // by -1.
    guidance.densityTarget = 0.0;
    guidance.catchinessTarget = 0.0;
    guidance.syncopationTarget = 0.0;
    guidance.octaveMovementTarget = 0.0;
    guidance.variationTarget = 0.0;
    guidance.repetitionTarget = 0.0;
    guidance.tensionSelectorTarget = 0.0;
    guidance.complexityTarget = 0.0;
    guidance.humanizationTarget = -1.0;
    guidance.noteLengthVariationTarget = -1.0;
    guidance.cadenceStrengthTarget = 0.75;
    guidance.valid = true;
    return guidance;
}



double averageMidiPitch(const Phrase& phrase)
{
    if (phrase.notes.empty())
        return 0.0;

    const double total = std::accumulate(
        phrase.notes.begin(),
        phrase.notes.end(),
        0.0,
        [](double sum, const NoteEvent& note)
        {
            return sum + static_cast<double>(note.midiNote);
        });

    return total / static_cast<double>(phrase.notes.size());
}

int offbeatCount(const Phrase& phrase)
{
    int count = 0;
    for (const auto& note : phrase.notes)
    {
        if (std::fmod(note.startBeat * 4.0, 2.0) >= 0.99)
            ++count;
    }
    return count;
}

int velocityDifferenceCount(const Phrase& a, const Phrase& b)
{
    const auto count = std::min(a.notes.size(), b.notes.size());
    int differences = 0;
    for (std::size_t i = 0; i < count; ++i)
        differences += a.notes[i].velocity != b.notes[i].velocity ? 1 : 0;
    return differences;
}

int timingDifferenceCount(const Phrase& a, const Phrase& b)
{
    const auto count = std::min(a.notes.size(), b.notes.size());
    int differences = 0;
    for (std::size_t i = 0; i < count; ++i)
    {
        differences += std::abs(a.notes[i].startBeat - b.notes[i].startBeat) > 1.0e-9 ? 1 : 0;
    }
    return differences;
}

bool noteIsInScale(const NoteEvent& note, const MusicalContext& context)
{
    const auto root = toPitchClass(context.key);
    const auto scale = context.scale.getPitchClasses(root);
    const auto pitchClass = (note.midiNote % 12 + 12) % 12;
    return std::find(scale.begin(), scale.end(), pitchClass) != scale.end();
}

void expectHardConstraints(const Phrase& phrase, const MusicalContext& context)
{
    expect(phrase.isValid(), "semantic selector output is valid");
    const int low = 12 * (context.parameters.octaveLow + 1);
    const int high = 12 * (context.parameters.octaveHigh + 1);

    for (const auto& note : phrase.notes)
    {
        expect(note.midiNote >= low && note.midiNote <= high,
               "AI selectors preserve configured octave range");
        expect(noteIsInScale(note, context),
               "AI selectors preserve key/scale");
    }

    expect(std::abs(phrase.lengthBeats - context.parameters.lengthBars * 4.0) < 1.0e-9,
           "AI selectors preserve phrase length");
}

void testGuidanceValidation()
{
    auto guidance = buildGuidance();
    expect(guidance.isValid(), "extended selector guidance is valid");

    guidance.densityTarget = 1.5;
    expect(!guidance.isValid(), "out-of-range selector guidance is rejected");
}

void testDensityInfluence()
{
    MusicalEngine engine;
    auto context = buildContext();
    auto low = buildGuidance();
    auto high = buildGuidance();
    low.densityTarget = -1.0;
    high.densityTarget = 1.0;

    const auto sparse = engine.generateWithAIGuidance(context, low, 12001);
    const auto dense = engine.generateWithAIGuidance(context, high, 12001);

    expect(dense.notes.size() > sparse.notes.size(),
           "higher Density produces more MIDI events");
    expectHardConstraints(sparse, context);
    expectHardConstraints(dense, context);
}

void testSyncopationInfluence()
{
    MusicalEngine engine;
    auto context = buildContext();
    auto straight = buildGuidance();
    auto syncopated = buildGuidance();
    straight.syncopationTarget = -1.0;
    syncopated.syncopationTarget = 1.0;

    const auto a = engine.generateWithAIGuidance(context, straight, 12002);
    const auto b = engine.generateWithAIGuidance(context, syncopated, 12002);

    expect(a.notes.size() == b.notes.size(),
           "syncopation does not silently change Density");
    expect(offbeatCount(a) != offbeatCount(b) || a.notes.front().startBeat != b.notes.front().startBeat,
           "higher Syncopation changes rhythmic placement");
    expectHardConstraints(b, context);
}

void testHumanizationInfluence()
{
    MusicalEngine engine;
    auto context = buildContext();
    auto dry = buildGuidance();
    auto human = buildGuidance();
    dry.humanizationTarget = -1.0;
    human.humanizationTarget = 1.0;

    const auto a = engine.generateWithAIGuidance(context, dry, 12003);
    const auto b = engine.generateWithAIGuidance(context, human, 12003);

    expect(velocityDifferenceCount(a, b) > 0 || timingDifferenceCount(a, b) > 0,
           "higher Humanization changes performance data");
    expectHardConstraints(b, context);
}

void testRepetitionInfluence()
{
    MusicalEngine engine;
    auto context = buildContext();
    auto low = buildGuidance();
    auto high = buildGuidance();
    low.repetitionTarget = -1.0;
    high.repetitionTarget = 1.0;

    const auto a = engine.generateWithAIGuidance(context, low, 12004);
    const auto b = engine.generateWithAIGuidance(context, high, 12004);

    bool different = a.notes.size() != b.notes.size();
    const auto count = std::min(a.notes.size(), b.notes.size());
    for (std::size_t i = 0; i < count && !different; ++i)
        different = a.notes[i].midiNote != b.notes[i].midiNote;

    expect(different, "Repetition selector changes motif reuse behavior");
    expectHardConstraints(b, context);
}



void testCatchinessInfluence()
{
    MusicalEngine engine;
    auto context = buildContext();
    auto low = buildGuidance();
    auto high = buildGuidance();
    low.catchinessTarget = -1.0;
    high.catchinessTarget = 1.0;

    const auto a = engine.generateWithAIGuidance(context, low, 12701);
    const auto b = engine.generateWithAIGuidance(context, high, 12701);

    expect(velocityDifferenceCount(a, b) > 0,
           "higher Catchiness changes accent/velocity behavior");
    expectHardConstraints(a, context);
    expectHardConstraints(b, context);
}

void testOctaveMovementInfluence()
{
    MusicalEngine engine;
    auto context = buildContext();
    auto low = buildGuidance();
    auto high = buildGuidance();
    low.octaveMovementTarget = -1.0;
    high.octaveMovementTarget = 1.0;

    const auto a = engine.generateWithAIGuidance(context, low, 12702);
    const auto b = engine.generateWithAIGuidance(context, high, 12702);

    bool different = a.notes.size() != b.notes.size();
    const auto count = std::min(a.notes.size(), b.notes.size());
    for (std::size_t i = 0; i < count && !different; ++i)
        different = a.notes[i].midiNote != b.notes[i].midiNote;

    expect(different,
           "higher Octave Movement changes register movement");
    expectHardConstraints(a, context);
    expectHardConstraints(b, context);
}

void testVariationInfluence()
{
    MusicalEngine engine;
    auto context = buildContext();
    auto low = buildGuidance();
    auto high = buildGuidance();
    low.variationTarget = -1.0;
    high.variationTarget = 1.0;

    const auto a = engine.generateWithAIGuidance(context, low, 12703);
    const auto b = engine.generateWithAIGuidance(context, high, 12703);

    bool different = a.notes.size() != b.notes.size();
    const auto count = std::min(a.notes.size(), b.notes.size());
    for (std::size_t i = 0; i < count && !different; ++i)
        different = a.notes[i].midiNote != b.notes[i].midiNote;

    expect(different,
           "higher Variation changes melodic material");
    expectHardConstraints(a, context);
    expectHardConstraints(b, context);
}

void testTensionInfluence()
{
    MusicalEngine engine;
    auto context = buildContext();
    auto low = buildGuidance();
    auto high = buildGuidance();
    low.tensionSelectorTarget = -1.0;
    high.tensionSelectorTarget = 1.0;

    const auto a = engine.generateWithAIGuidance(context, low, 12704);
    const auto b = engine.generateWithAIGuidance(context, high, 12704);

    bool different = a.notes.size() != b.notes.size();
    const auto count = std::min(a.notes.size(), b.notes.size());
    for (std::size_t i = 0; i < count && !different; ++i)
        different = a.notes[i].midiNote != b.notes[i].midiNote;

    expect(different,
           "higher Tension changes melodic tension behavior");
    expectHardConstraints(a, context);
    expectHardConstraints(b, context);
}

void testComplexityInfluence()
{
    MusicalEngine engine;
    auto context = buildContext();
    auto low = buildGuidance();
    auto high = buildGuidance();
    low.complexityTarget = -1.0;
    high.complexityTarget = 1.0;

    const auto a = engine.generateWithAIGuidance(context, low, 12705);
    const auto b = engine.generateWithAIGuidance(context, high, 12705);

    bool different = a.notes.size() != b.notes.size();
    const auto count = std::min(a.notes.size(), b.notes.size());
    for (std::size_t i = 0; i < count && !different; ++i)
        different = a.notes[i].midiNote != b.notes[i].midiNote;

    expect(different,
           "higher Complexity changes compositional pitch choices");
    expectHardConstraints(a, context);
    expectHardConstraints(b, context);
}

void testNoteLengthVariationInfluence()
{
    MusicalEngine engine;
    auto context = buildContext();
    auto low = buildGuidance();
    auto high = buildGuidance();
    low.noteLengthVariationTarget = -1.0;
    high.noteLengthVariationTarget = 1.0;

    const auto a = engine.generateWithAIGuidance(context, low, 12706);
    const auto b = engine.generateWithAIGuidance(context, high, 12706);

    bool different = a.notes.size() != b.notes.size();
    const auto count = std::min(a.notes.size(), b.notes.size());
    for (std::size_t i = 0; i < count && !different; ++i)
        different = std::abs(a.notes[i].durationBeats - b.notes[i].durationBeats) > 1.0e-9;

    expect(different,
           "higher Length Variation changes note durations");
    expectHardConstraints(a, context);
    expectHardConstraints(b, context);
}

void testCadenceStrengthInfluence()
{
    MusicalEngine engine;
    auto context = buildContext();
    auto weak = buildGuidance();
    auto strong = buildGuidance();
    weak.cadenceStrengthTarget = 0.0;
    strong.cadenceStrengthTarget = 1.0;

    const auto a = engine.generateWithAIGuidance(context, weak, 12707);
    const auto b = engine.generateWithAIGuidance(context, strong, 12707);

    expect(a.notes.empty() || b.notes.empty() ||
           a.notes.back().midiNote != b.notes.back().midiNote ||
           std::abs(a.notes.back().durationBeats - b.notes.back().durationBeats) > 1.0e-9,
           "higher Cadence Strength changes phrase-ending resolution");
    expectHardConstraints(a, context);
    expectHardConstraints(b, context);
}

void testNoteLengthSelectorInfluence()
{
    MusicalEngine engine;
    auto context = buildContext();
    context.parameters.lengthBars = 4;
    context.parameters.phraseLengthBars = 4;
    context.parameters.density = 100;
    context.parameters.cadenceStrength = 0;

    auto shortContext = context;
    auto longContext = context;
    shortContext.parameters.noteLength = NoteLength::Short;
    longContext.parameters.noteLength = NoteLength::Long;

    const auto shortPhrase =
        engine.generateWithAIGuidance(
            shortContext,
            buildGuidance(),
            12901);
    const auto longPhrase =
        engine.generateWithAIGuidance(
            longContext,
            buildGuidance(),
            12901);

    expect(
        shortPhrase.notes.size() == longPhrase.notes.size(),
        "Note Length preserves event topology");

    bool durationDiffers = false;
    const auto count = std::min(
        shortPhrase.notes.size(),
        longPhrase.notes.size());

    for (std::size_t i = 0; i < count; ++i)
    {
        if (std::abs(
                shortPhrase.notes[i].durationBeats -
                longPhrase.notes[i].durationBeats) > 1.0e-9)
        {
            durationDiffers = true;
            break;
        }
    }

    expect(
        durationDiffers,
        "Note Length changes generated note durations");

    expectHardConstraints(shortPhrase, shortContext);
    expectHardConstraints(longPhrase, longContext);
}

void testPhraseContourSelectorInfluence()
{
    MusicalEngine engine;
    auto context = buildContext();
    context.parameters.lengthBars = 8;
    context.parameters.phraseLengthBars = 8;
    context.parameters.density = 100;
    context.parameters.octaveLow = 0;
    context.parameters.octaveHigh = 6;
    context.parameters.octaveMovement = 0;
    context.parameters.octaveShift = 0;
    context.parameters.cadenceStrength = 0;

    auto ascendingContext = context;
    auto descendingContext = context;
    ascendingContext.parameters.phraseContour = PhraseContour::Ascending;
    descendingContext.parameters.phraseContour = PhraseContour::Descending;

    const auto ascending =
        engine.generateWithAIGuidance(
            ascendingContext,
            buildGuidance(),
            12902);
    const auto descending =
        engine.generateWithAIGuidance(
            descendingContext,
            buildGuidance(),
            12902);

    const bool averageDiffers =
        std::abs(
            averageMidiPitch(ascending) -
            averageMidiPitch(descending)) > 0.5;

    const bool endpointDiffers =
        ascending.notes.empty() ||
        descending.notes.empty() ||
        ascending.notes.back().midiNote !=
            descending.notes.back().midiNote;

    expect(
        averageDiffers || endpointDiffers,
        "Phrase Contour changes register trajectory");

    expectHardConstraints(ascending, ascendingContext);
    expectHardConstraints(descending, descendingContext);
}

void testOctaveShiftSelectorInfluence()
{
    MusicalEngine engine;
    auto context = buildContext();
    context.parameters.lengthBars = 8;
    context.parameters.phraseLengthBars = 8;
    context.parameters.density = 100;
    context.parameters.octaveLow = 0;
    context.parameters.octaveHigh = 6;
    context.parameters.octaveMovement = 0;
    context.parameters.phraseContour = PhraseContour::Flat;
    context.parameters.cadenceStrength = 0;

    auto lowerContext = context;
    auto upperContext = context;
    lowerContext.parameters.octaveShift = -2;
    upperContext.parameters.octaveShift = 2;

    const auto lower =
        engine.generateWithAIGuidance(
            lowerContext,
            buildGuidance(),
            12903);
    const auto upper =
        engine.generateWithAIGuidance(
            upperContext,
            buildGuidance(),
            12903);

    expect(
        std::abs(
            averageMidiPitch(lower) -
            averageMidiPitch(upper)) >= 6.0,
        "Octave Shift changes generated register");

    expectHardConstraints(lower, lowerContext);
    expectHardConstraints(upper, upperContext);
}

void testLengthSelectorInfluence()
{
    MusicalEngine engine;
    auto shortContext = buildContext();
    auto longContext = shortContext;

    shortContext.parameters.lengthBars = 4;
    longContext.parameters.lengthBars = 8;
    shortContext.parameters.phraseLengthBars = 4;
    longContext.parameters.phraseLengthBars = 4;

    const auto shortPhrase =
        engine.generateWithAIGuidance(
            shortContext,
            buildGuidance(),
            12904);
    const auto longPhrase =
        engine.generateWithAIGuidance(
            longContext,
            buildGuidance(),
            12904);

    expect(
        std::abs(shortPhrase.lengthBeats - 16.0) < 1.0e-9,
        "Length selector produces four-bar phrase length");

    expect(
        std::abs(longPhrase.lengthBeats - 32.0) < 1.0e-9,
        "Length selector produces eight-bar phrase length");

    expect(
        longPhrase.lengthBeats > shortPhrase.lengthBeats,
        "higher Length produces a longer phrase");

    expectHardConstraints(shortPhrase, shortContext);
    expectHardConstraints(longPhrase, longContext);
}

void testCadenceStyleSelectorInfluence()
{
    MusicalEngine engine;
    auto context = buildContext();
    context.parameters.lengthBars = 4;
    context.parameters.phraseLengthBars = 4;
    context.parameters.density = 100;
    context.parameters.tension = 100;
    context.parameters.complexity = 100;
    context.parameters.cadenceStrength = 100;

    auto rootContext = context;
    auto fifthContext = context;
    rootContext.parameters.cadenceStyle = CadenceStyle::Root;
    fifthContext.parameters.cadenceStyle = CadenceStyle::Fifth;

    bool foundStyleDifference = false;

    for (std::uint32_t seed = 12910; seed < 12926; ++seed)
    {
        const auto rootPhrase =
            engine.generateWithAIGuidance(
                rootContext,
                buildGuidance(),
                seed);
        const auto fifthPhrase =
            engine.generateWithAIGuidance(
                fifthContext,
                buildGuidance(),
                seed);

        expectHardConstraints(rootPhrase, rootContext);
        expectHardConstraints(fifthPhrase, fifthContext);

        if (!rootPhrase.notes.empty() &&
            !fifthPhrase.notes.empty() &&
            (rootPhrase.notes.back().midiNote !=
                 fifthPhrase.notes.back().midiNote ||
             rootPhrase.notes.back().durationBeats !=
                 fifthPhrase.notes.back().durationBeats))
        {
            foundStyleDifference = true;
            break;
        }
    }

    expect(
        foundStyleDifference,
        "Cadence Style changes phrase-ending resolution");
}


void testHumanizationPreservesMusicalContent()
{
    MusicalEngine engine;
    auto context = buildContext();
    context.parameters.density = 75;
    context.parameters.humanization = 0;

    auto dryContext = context;
    auto humanizedContext = context;
    humanizedContext.parameters.humanization = 100;

    const auto dry = engine.generateWithAIGuidance(
        dryContext, buildGuidance(), 13001);
    const auto humanized = engine.generateWithAIGuidance(
        humanizedContext, buildGuidance(), 13001);

    expect(
        dry.notes.size() == humanized.notes.size(),
        "Humanization preserves event count");

    const auto count = std::min(dry.notes.size(), humanized.notes.size());
    for (std::size_t i = 0; i < count; ++i)
    {
        expect(
            dry.notes[i].midiNote == humanized.notes[i].midiNote,
            "Humanization does not alter pitch content");
        expect(
            std::abs(dry.notes[i].durationBeats - humanized.notes[i].durationBeats) <= 1.0e-9,
            "Humanization does not alter note durations");
    }

    expectHardConstraints(dry, dryContext);
    expectHardConstraints(humanized, humanizedContext);
}

void testNoteLengthPreservesRhythmicPlacement()
{
    MusicalEngine engine;
    auto context = buildContext();
    context.parameters.density = 80;

    auto shortContext = context;
    auto longContext = context;
    shortContext.parameters.noteLength = NoteLength::Short;
    longContext.parameters.noteLength = NoteLength::Long;

    const auto shortPhrase = engine.generateWithAIGuidance(
        shortContext, buildGuidance(), 13002);
    const auto longPhrase = engine.generateWithAIGuidance(
        longContext, buildGuidance(), 13002);

    expect(
        shortPhrase.notes.size() == longPhrase.notes.size(),
        "Note Length preserves event count");

    const auto count = std::min(
        shortPhrase.notes.size(),
        longPhrase.notes.size());

    bool durationChanged = false;
    for (std::size_t i = 0; i < count; ++i)
    {
        expect(
            std::abs(shortPhrase.notes[i].startBeat - longPhrase.notes[i].startBeat) <= 1.0e-9,
            "Note Length preserves rhythmic placement");
        expect(
            shortPhrase.notes[i].midiNote == longPhrase.notes[i].midiNote,
            "Note Length preserves pitch content");

        durationChanged = durationChanged ||
            std::abs(shortPhrase.notes[i].durationBeats - longPhrase.notes[i].durationBeats) > 1.0e-9;
    }

    expect(durationChanged, "Note Length changes duration semantics");
    expectHardConstraints(shortPhrase, shortContext);
    expectHardConstraints(longPhrase, longContext);
}

void testOctaveShiftPreservesRhythm()
{
    MusicalEngine engine;
    auto context = buildContext();
    context.parameters.lengthBars = 8;
    context.parameters.phraseLengthBars = 8;
    context.parameters.density = 80;
    context.parameters.octaveLow = 0;
    context.parameters.octaveHigh = 6;
    context.parameters.octaveMovement = 0;
    context.parameters.phraseContour = PhraseContour::Flat;
    context.parameters.cadenceStrength = 0;

    auto lowerContext = context;
    auto upperContext = context;
    lowerContext.parameters.octaveShift = -2;
    upperContext.parameters.octaveShift = 2;

    const auto lower = engine.generateWithAIGuidance(
        lowerContext, buildGuidance(), 13003);
    const auto upper = engine.generateWithAIGuidance(
        upperContext, buildGuidance(), 13003);

    expect(lower.notes.size() == upper.notes.size(),
           "Octave Shift preserves event count");

    const auto count = std::min(lower.notes.size(), upper.notes.size());
    for (std::size_t i = 0; i < count; ++i)
    {
        expect(
            std::abs(lower.notes[i].startBeat - upper.notes[i].startBeat) <= 1.0e-9,
            "Octave Shift preserves rhythmic placement");
        expect(
            std::abs(lower.notes[i].durationBeats - upper.notes[i].durationBeats) <= 1.0e-9,
            "Octave Shift preserves note durations");
    }

    expect(
        std::abs(averageMidiPitch(lower) - averageMidiPitch(upper)) >= 6.0,
        "Octave Shift changes register semantics");

    expectHardConstraints(lower, lowerContext);
    expectHardConstraints(upper, upperContext);
}

void testSelectorInteractionPreservesHardConstraints()
{
    MusicalEngine engine;
    auto context = buildContext();
    context.parameters.lengthBars = 8;
    context.parameters.phraseLengthBars = 8;
    context.parameters.octaveLow = -1;
    context.parameters.octaveHigh = 5;
    context.parameters.density = 100;
    context.parameters.syncopation = 100;
    context.parameters.octaveMovement = 100;
    context.parameters.variation = 100;
    context.parameters.repetition = 0;
    context.parameters.tension = 100;
    context.parameters.complexity = 100;
    context.parameters.humanization = 100;
    context.parameters.noteLengthVariation = 100;
    context.parameters.cadenceStrength = 100;
    context.parameters.cadenceStyle = CadenceStyle::Fifth;
    context.parameters.phraseContour = PhraseContour::Ascending;
    context.parameters.octaveShift = 2;
    context.parameters.noteLength = NoteLength::Short;

    const auto phrase = engine.generateWithAIGuidance(
        context, buildGuidance(), 13004);

    expectHardConstraints(phrase, context);
}

void testAllSelectorDimensionsRemainBounded()
{
    MusicalEngine engine;
    auto context = buildContext();
    auto guidance = buildGuidance();
    guidance.densityTarget = 1.0;
    guidance.catchinessTarget = 1.0;
    guidance.syncopationTarget = 1.0;
    guidance.octaveMovementTarget = 1.0;
    guidance.variationTarget = 1.0;
    guidance.repetitionTarget = 1.0;
    guidance.tensionSelectorTarget = 1.0;
    guidance.complexityTarget = 1.0;
    guidance.humanizationTarget = 1.0;
    guidance.noteLengthVariationTarget = 1.0;
    guidance.cadenceStrengthTarget = 1.0;

    const auto phrase = engine.generateWithAIGuidance(context, guidance, 12005);
    expectHardConstraints(phrase, context);
}

} // namespace

int main()
{
    testGuidanceValidation();
    testDensityInfluence();
    testSyncopationInfluence();
    testHumanizationInfluence();
    testRepetitionInfluence();
    testCatchinessInfluence();
    testOctaveMovementInfluence();
    testVariationInfluence();
    testTensionInfluence();
    testComplexityInfluence();
    testNoteLengthVariationInfluence();
    testCadenceStrengthInfluence();
    testNoteLengthSelectorInfluence();
    testPhraseContourSelectorInfluence();
    testOctaveShiftSelectorInfluence();
    testLengthSelectorInfluence();
    testCadenceStyleSelectorInfluence();
    testHumanizationPreservesMusicalContent();
    testNoteLengthPreservesRhythmicPlacement();
    testOctaveShiftPreservesRhythm();
    testSelectorInteractionPreservesHardConstraints();
    testAllSelectorDimensionsRemainBounded();

    std::cout << "MIDI-GenGX Phase 130 selector semantic invariants tests passed.\n";
    return 0;
}
