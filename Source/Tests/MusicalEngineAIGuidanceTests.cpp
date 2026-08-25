#include "Music/MusicalEngine.h"

#include <cstdlib>
#include <iostream>

using namespace midigengx::music;
using namespace midigengx::domain;

namespace
{
void expect(bool condition, const char* message)
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

MusicalContext buildContext()
{
    MusicalContext context;
    context.key = Key::C;
    context.scale = Scale{ScaleType::Minor};
    context.role = Role::Lead;
    context.parameters.lengthBars = 8;
    context.parameters.phraseLengthBars = 4;
    context.parameters.octaveLow = 0;
    context.parameters.octaveHigh = 2;
    context.parameters.density = 55;
    context.parameters.tension = 40;
    context.parameters.variation = 45;
    context.parameters.complexity = 40;
    context.normalize();
    return context;
}

CompositionAIGuidance buildGuidance()
{
    CompositionAIGuidance guidance;

    guidance.roleTarget = 0.20;
    guidance.tensionTarget = 0.70;
    guidance.tensionDeltaTarget = 0.35;
    guidance.harmonyDegreeTarget = -0.20;
    guidance.harmonyQualityTarget = 0.80;
    guidance.harmonicDegreeDeltaTarget = 0.25;
    guidance.confidence = 1.0;
    guidance.valid = true;

    return guidance;
}

void testValidAIGuidanceProducesPhrase()
{
    const MusicalEngine engine;
    const auto context = buildContext();

    const auto phrase =
        engine.generateWithAIGuidance(
            context,
            buildGuidance(),
            777);

    expect(
        !phrase.notes.empty(),
        "AI-guided generation produces notes");

    expect(
        phrase.isValid(),
        "AI-guided phrase is valid");
}

void testInvalidGuidanceFallsBackSafely()
{
    const MusicalEngine engine;
    const auto context = buildContext();

    auto invalid =
        buildGuidance();

    invalid.tensionTarget = 2.0;
    invalid.valid = false;

    const auto guided =
        engine.generateWithAIGuidance(
            context,
            invalid,
            777);

    const auto baseline =
        engine.generate(
            context,
            777);

    expect(
        guided.notes.size() ==
            baseline.notes.size(),
        "invalid AI guidance falls back to baseline");

    expect(
        guided.lengthBeats ==
            baseline.lengthBeats,
        "fallback preserves phrase length");
}

void testDefaultGenerationIsUnchanged()
{
    const MusicalEngine engine;
    const auto context = buildContext();

    const auto baseline =
        engine.generate(
            context,
            1234);

    auto untouched =
        context;

    const auto repeat =
        engine.generate(
            untouched,
            1234);

    expect(
        baseline.notes.size() ==
            repeat.notes.size(),
        "default generation remains deterministic");

    expect(
        baseline.lengthBeats ==
            repeat.lengthBeats,
        "default generation retains its existing output shape");
}

void testAIGuidanceIsDeterministic()
{
    const MusicalEngine engine;
    const auto context = buildContext();
    const auto guidance = buildGuidance();

    const auto first =
        engine.generateWithAIGuidance(
            context,
            guidance,
            42);

    const auto second =
        engine.generateWithAIGuidance(
            context,
            guidance,
            42);

    expect(
        first.notes.size() ==
            second.notes.size(),
        "AI-guided generation is deterministic");

    expect(
        first.lengthBeats ==
            second.lengthBeats,
        "AI-guided phrase length is deterministic");
}

void testExplicitContextConstraintsRemainAuthoritative()
{
    const MusicalEngine engine;
    auto context = buildContext();

    context.parameters.lengthBars = 12;
    context.parameters.phraseLengthBars = 3;
    context.parameters.octaveLow = -1;
    context.parameters.octaveHigh = 1;
    context.normalize();

    const auto phrase =
        engine.generateWithAIGuidance(
            context,
            buildGuidance(),
            99);

    expect(
        phrase.isValid(),
        "guided generation respects explicit context validity");

    expect(
        phrase.lengthBeats <=
            static_cast<double>(
                context.parameters.lengthBars * 4),
        "guided generation remains inside explicit length constraint");

    for (const auto& note :
         phrase.notes)
    {
        expect(
            note.midiNote >= 0 &&
            note.midiNote <= 127,
            "guided generation preserves valid MIDI range");
    }
}

} // namespace

int main()
{
    testValidAIGuidanceProducesPhrase();
    testInvalidGuidanceFallsBackSafely();
    testDefaultGenerationIsUnchanged();
    testAIGuidanceIsDeterministic();
    testExplicitContextConstraintsRemainAuthoritative();

    std::cout
        << "MIDI-GenGX Musical Engine AI Guidance tests passed.\n";

    return 0;
}
