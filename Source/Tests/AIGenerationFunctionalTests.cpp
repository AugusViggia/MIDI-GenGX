#include "Music/CompositionAIGenerationCoordinator.h"
#include "Music/CompositionNeuralTrainer.h"
#include "Music/MotifDevelopment.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <set>
#include <string>
#include <vector>

using namespace midigengx::music;
using namespace midigengx::domain;

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

MusicalContext buildContext(
    Role role,
    Key key,
    ScaleType scaleType,
    int octaveLow,
    int octaveHigh)
{
    MusicalContext context;

    context.key = key;
    context.scale = Scale{scaleType};
    context.role = role;

    context.parameters.lengthBars = 8;
    context.parameters.phraseLengthBars = 4;
    context.parameters.octaveLow = octaveLow;
    context.parameters.octaveHigh = octaveHigh;
    context.parameters.density = 55;
    context.parameters.tension = 45;
    context.parameters.variation = 50;
    context.parameters.complexity = 45;
    context.parameters.syncopation = 45;
    context.parameters.catchiness = 55;
    context.normalize();

    return context;
}

Motif makeMotif()
{
    Motif motif;
    motif.lengthBeats = 2.0;
    motif.notes =
    {
        MotifNote{0.0, 0.5, 0, 0},
        MotifNote{0.5, 0.5, 2, 1},
        MotifNote{1.0, 0.5, 5, -1},
        MotifNote{1.5, 0.5, 3, 0}
    };
    return motif;
}

CompositionDatasetPreparedView buildPrepared()
{
    MusicalContext context;
    context.parameters.lengthBars = 16;
    context.parameters.phraseLengthBars = 4;
    context.normalize();

    const auto structure =
        planPhraseStructure(context);

    const auto harmony =
        planHarmony(
            context,
            structure);

    const auto graph =
        buildCompositionKnowledgeGraph(
            structure,
            harmony);

    const auto transitions =
        analyzeCompositionTransitions(
            graph);

    const auto motifGraph =
        buildMotifOccurrenceGraph(
            {
                makeMotif(),
                MotifDevelopment::transpose(
                    makeMotif(),
                    7)
            },
            {0, 8});

    const auto motifProfile =
        analyzeMotifRecurrence(
            motifGraph);

    const auto catalog =
        buildMotifKnowledgeCatalog(
            motifProfile);

    const auto composition =
        buildCompositionKnowledgeRecord(
            structure,
            harmony,
            catalog);

    const auto snapshot =
        buildCompositionKnowledgeSnapshot(
            composition,
            graph,
            transitions);

    std::vector<CompositionDatasetInput>
        inputs;

    for (int index = 0;
         index < 24;
         ++index)
    {
        inputs.push_back(
        {
            "functional-" +
                std::to_string(index),
            snapshot
        });
    }

    const auto dataset =
        buildCompositionDataset(
            inputs);

    const auto quality =
        assessCompositionDatasetQuality(
            dataset);

    const auto partition =
        buildCompositionDatasetPartition(
            dataset,
            0.20,
            0.20);

    const auto manifest =
        buildCompositionDatasetManifest(
            dataset,
            quality,
            partition);

    return prepareCompositionDatasetForLearning(
        dataset,
        quality,
        manifest,
        partition);
}

CompositionAIGenerationCoordinator
buildCoordinator()
{
    const auto prepared =
        buildPrepared();

    const auto contract =
        buildCompositionLearningContract(
            prepared,
            LearningObjective::NextSectionPrediction);

    auto model =
        initializeCompositionNeuralModel(
            contract);

    CompositionNeuralTrainingConfig config;
    config.epochs = 16;
    config.learningRate = 0.001;
    config.optimizer =
        CompositionNeuralOptimizer::Adam;

    const auto training =
        trainCompositionNeuralModel(
            model,
            prepared,
            config);

    expect(
        training.isValid(),
        "functional test model training is valid");

    const auto pipeline =
        buildCompositionInferencePipeline(
            model);

    const auto bridge =
        buildCompositionAIEngineBridge(
            true);

    return buildCompositionAIGenerationCoordinator(
        pipeline,
        bridge,
        true);
}

CompositionAIGenerationRequest buildRequest(
    const MusicalContext& context,
    std::uint32_t seed)
{
    CompositionAIGenerationRequest request;

    request.context = context;

    request.globalFeatures =
    {
        0.25, 0.50, 0.20, 0.30, 0.10, 0.20,
        0.45, 0.20, 0.80, 0.30, 0.15, 0.10,
        0.70
    };

    request.contextSectionFeatures =
    {
        0.00, 0.45, 0.10,
        0.25, 0.20, 0.05
    };

    request.contextIsValid = true;
    request.seed = seed;

    return request;
}

void expectValidPhrase(
    const Phrase& phrase,
    const MusicalContext& context,
    const char* message)
{
    expect(
        phrase.isValid(),
        message);

    expect(
        phrase.lengthBeats >= 0.0 &&
        phrase.lengthBeats <=
            static_cast<double>(
                context.parameters.lengthBars * 4),
        "generated phrase remains inside length bounds");

    for (const auto& note :
         phrase.notes)
    {
        expect(
            note.midiNote >= 0 &&
            note.midiNote <= 127,
            "generated note remains inside MIDI range");

        expect(
            std::isfinite(note.startBeat) &&
            std::isfinite(note.durationBeats) &&
            std::isfinite(note.velocity),
            "generated note fields remain finite");

        expect(
            note.durationBeats > 0.0,
            "generated note duration remains positive");
    }
}

void testRoles()
{
    const auto coordinator =
        buildCoordinator();

    const Role roles[] =
    {
        Role::Lead,
        Role::Melody,
        Role::Pluck,
        Role::Piano,
        Role::Sequence,
        Role::Bass,
        Role::Arp,
        Role::Chords,
        Role::Pad
    };

    for (const auto role :
         roles)
    {
        const auto context =
            buildContext(
                role,
                Key::C,
                ScaleType::Minor,
                0,
                2);

        const auto result =
            coordinator.generate(
                buildRequest(
                    context,
                    100 + static_cast<std::uint32_t>(role)));

        expect(
            result.isValid(),
            "every supported role produces a valid AI-guided phrase");

        expect(
            result.usedAI,
            "role generation uses AI");
    }
}

void testKeysAndScales()
{
    const auto coordinator =
        buildCoordinator();

    const Key keys[] =
    {
        Key::C,
        Key::D,
        Key::E,
        Key::F,
        Key::G,
        Key::A,
        Key::B
    };

    const ScaleType scales[] =
    {
        ScaleType::Major,
        ScaleType::Minor
    };

    for (const auto scale :
         scales)
    {
        for (const auto key :
             keys)
        {
            const auto context =
                buildContext(
                    Role::Lead,
                    key,
                    scale,
                    -1,
                    2);

            const auto result =
                coordinator.generate(
                    buildRequest(
                        context,
                        300 +
                            static_cast<std::uint32_t>(
                                static_cast<int>(key) +
                                static_cast<int>(scale) * 16)));

            expect(
                result.isValid(),
                "key/scale combination produces valid generation");

            expect(
                result.usedAI,
                "key/scale combination uses AI");
        }
    }
}

void testSeedDeterminism()
{
    const auto coordinator =
        buildCoordinator();

    const auto context =
        buildContext(
            Role::Lead,
            Key::C,
            ScaleType::Minor,
            0,
            2);

    const auto first =
        coordinator.generate(
            buildRequest(
                context,
                9001));

    const auto second =
        coordinator.generate(
            buildRequest(
                context,
                9001));

    expect(
        first.isValid() &&
        second.isValid(),
        "determinism cases produce valid phrases");

    expect(
        first.usedAI &&
        second.usedAI,
        "determinism cases use AI");

    expect(
        first.phrase.notes.size() ==
            second.phrase.notes.size(),
        "same seed produces same note count");

    expect(
        first.phrase.lengthBeats ==
            second.phrase.lengthBeats,
        "same seed produces same phrase length");

    for (std::size_t i = 0;
         i < first.phrase.notes.size();
         ++i)
    {
        const auto& a =
            first.phrase.notes[i];
        const auto& b =
            second.phrase.notes[i];

        expect(
            a.midiNote == b.midiNote &&
            a.startBeat == b.startBeat &&
            a.durationBeats == b.durationBeats &&
            a.velocity == b.velocity,
            "same seed produces identical MIDI events");
    }
}

void testDifferentSeedsRemainFunctional()
{
    const auto coordinator =
        buildCoordinator();

    const auto context =
        buildContext(
            Role::Lead,
            Key::C,
            ScaleType::Minor,
            0,
            2);

    const auto first =
        coordinator.generate(
            buildRequest(
                context,
                10001));

    const auto second =
        coordinator.generate(
            buildRequest(
                context,
                10002));

    expect(
        first.isValid() &&
        second.isValid(),
        "different seeds both produce valid phrases");

    expect(
        first.usedAI &&
        second.usedAI,
        "different seeds use AI");

    bool differs = false;

    if (first.phrase.notes.size() !=
        second.phrase.notes.size())
    {
        differs = true;
    }
    else
    {
        for (std::size_t i = 0;
             i < first.phrase.notes.size();
             ++i)
        {
            if (first.phrase.notes[i].midiNote !=
                    second.phrase.notes[i].midiNote ||
                first.phrase.notes[i].startBeat !=
                    second.phrase.notes[i].startBeat)
            {
                differs = true;
                break;
            }
        }
    }

    expect(
        differs,
        "different seeds can produce different musical material");
}

void testAIOnVsOff()
{
    const auto enabled =
        buildCoordinator();

    const auto disabled =
        buildCompositionAIGenerationCoordinator(
            enabled.pipeline,
            enabled.bridge,
            false);

    const auto context =
        buildContext(
            Role::Lead,
            Key::C,
            ScaleType::Minor,
            0,
            2);

    const auto request =
        buildRequest(
            context,
            7007);

    const auto ai =
        enabled.generate(
            request);

    const auto baseline =
        disabled.generate(
            request);

    expect(
        ai.isValid() &&
        baseline.isValid(),
        "AI on/off both produce valid phrases");

    expect(
        ai.usedAI,
        "AI enabled path reports AI usage");

    expect(
        !baseline.usedAI,
        "AI disabled path reports baseline usage");
}

void testGuidanceExtremesStaySafe()
{
    const MusicalEngine engine;

    const auto context =
        buildContext(
            Role::Lead,
            Key::C,
            ScaleType::Minor,
            -2,
            2);

    CompositionAIGuidance guidance;

    guidance.roleTarget = 1.0;
    guidance.tensionTarget = 1.0;
    guidance.tensionDeltaTarget = 1.0;
    guidance.harmonyDegreeTarget = -1.0;
    guidance.harmonyQualityTarget = 1.0;
    guidance.harmonicDegreeDeltaTarget = -1.0;
    guidance.confidence = 1.0;
    guidance.valid = true;

    const auto result =
        engine.generateWithAIGuidance(
            context,
            guidance,
            44);

    expectValidPhrase(
        result,
        context,
        "extreme AI guidance remains safe");
}

void testTightRegisterIsSafe()
{
    const auto coordinator =
        buildCoordinator();

    const auto context =
        buildContext(
            Role::Lead,
            Key::G,
            ScaleType::Major,
            1,
            1);

    const auto result =
        coordinator.generate(
            buildRequest(
                context,
                5005));

    expectValidPhrase(
        result.phrase,
        context,
        "tight octave range produces safe output");
}

void testInvalidAIRequestDoesNotGenerate()
{
    const auto coordinator =
        buildCoordinator();

    auto request =
        buildRequest(
            buildContext(
                Role::Lead,
                Key::C,
                ScaleType::Minor,
                0,
                2),
            123);

    request.globalFeatures.clear();
    request.contextSectionFeatures.clear();
    request.contextIsValid = false;

    const auto result =
        coordinator.generate(
            request);

    expect(
        !result.valid,
        "invalid AI request is rejected");
}

void testDisabledAIBypassesFeatureValidation()
{
    const auto enabled =
        buildCoordinator();

    const auto disabled =
        buildCompositionAIGenerationCoordinator(
            enabled.pipeline,
            enabled.bridge,
            false);

    auto request =
        buildRequest(
            buildContext(
                Role::Bass,
                Key::A,
                ScaleType::Minor,
                -2,
                0),
            222);

    request.globalFeatures.clear();
    request.contextSectionFeatures.clear();
    request.contextIsValid = false;

    const auto result =
        disabled.generate(
            request);

    expect(
        result.valid,
        "disabled AI falls back without ML feature vectors");

    expect(
        !result.usedAI,
        "disabled AI reports baseline generation");
}

void testMusicalContextConstraintsRemainAuthoritative()
{
    const auto coordinator =
        buildCoordinator();

    const auto context =
        buildContext(
            Role::Lead,
            Key::D,
            ScaleType::Major,
            -1,
            1);

    const auto result =
        coordinator.generate(
            buildRequest(
                context,
                88));

    expectValidPhrase(
        result.phrase,
        context,
        "explicit musical constraints remain authoritative");

    expect(
        result.phrase.lengthBeats <=
            static_cast<double>(
                context.parameters.lengthBars * 4),
        "explicit phrase length remains authoritative");
}

} // namespace

int main()
{
    testRoles();
    testKeysAndScales();
    testSeedDeterminism();
    testDifferentSeedsRemainFunctional();
    testAIOnVsOff();
    testGuidanceExtremesStaySafe();
    testTightRegisterIsSafe();
    testInvalidAIRequestDoesNotGenerate();
    testDisabledAIBypassesFeatureValidation();
    testMusicalContextConstraintsRemainAuthoritative();

    std::cout
        << "MIDI-GenGX AI Generation Functional tests passed.\n";

    return 0;
}
