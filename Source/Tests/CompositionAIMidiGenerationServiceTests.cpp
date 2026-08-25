#include "Music/CompositionAIMidiGenerationService.h"
#include "Domain/AbletonOctaveConvention.h"
#include "Music/CompositionMidiCorpusTrainingService.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <vector>

using namespace midigengx::music;

namespace
{
namespace fs = std::filesystem;

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

void appendU32BE(
    std::vector<std::uint8_t>& b,
    std::uint32_t v)
{
    b.push_back(static_cast<std::uint8_t>(v >> 24));
    b.push_back(static_cast<std::uint8_t>(v >> 16));
    b.push_back(static_cast<std::uint8_t>(v >> 8));
    b.push_back(static_cast<std::uint8_t>(v));
}

std::vector<std::uint8_t> buildMidi(int root)
{
    std::vector<std::uint8_t> bytes{'M','T','h','d'};
    appendU32BE(bytes, 6);
    bytes.push_back(0); bytes.push_back(0);
    bytes.push_back(0); bytes.push_back(1);
    bytes.push_back(0x01); bytes.push_back(0xE0);

    constexpr int chord[3] = {0,4,7};
    std::vector<std::uint8_t> track;

    for (int beat = 0; beat < 64; ++beat)
    {
        for (int note = 0; note < 3; ++note)
        {
            track.push_back(0x00);
            track.push_back(0x90);
            track.push_back(
                static_cast<std::uint8_t>(
                    root + chord[note]));
            track.push_back(96);
        }

        for (int note = 0; note < 3; ++note)
        {
            if (note == 0)
            {
                track.push_back(0x83);
                track.push_back(0x60);
            }
            else
            {
                track.push_back(0x00);
            }

            track.push_back(0x80);
            track.push_back(
                static_cast<std::uint8_t>(
                    root + chord[note]));
            track.push_back(0);
        }
    }

    track.push_back(0x00);
    track.push_back(0xFF);
    track.push_back(0x2F);
    track.push_back(0x00);

    bytes.push_back('M'); bytes.push_back('T');
    bytes.push_back('r'); bytes.push_back('k');
    appendU32BE(
        bytes,
        static_cast<std::uint32_t>(
            track.size()));

    bytes.insert(
        bytes.end(),
        track.begin(),
        track.end());

    return bytes;
}

fs::path createCorpus(
    std::size_t count)
{
    const auto root =
        fs::temp_directory_path() /
        "MIDI_GenGX_Phase82_Corpus";

    std::error_code error;
    fs::remove_all(root,error);
    fs::create_directories(root,error);

    for (std::size_t index = 0;
         index < count;
         ++index)
    {
        const auto bytes =
            buildMidi(
                60 +
                static_cast<int>(
                    index % 5));

        std::ofstream file(
            root /
                ("ai-midi-" +
                 std::to_string(index) +
                 ".mid"),
            std::ios::binary);

        file.write(
            reinterpret_cast<const char*>(
                bytes.data()),
            static_cast<std::streamsize>(
                bytes.size()));
    }

    return root;
}

void cleanup(
    const fs::path& root)
{
    std::error_code error;
    fs::remove_all(
        root,
        error);
}

CompositionNeuralTrainingConfig config()
{
    CompositionNeuralTrainingConfig value;
    value.epochs = 8;
    value.learningRate = 0.001;
    value.optimizer =
        CompositionNeuralOptimizer::Adam;
    value.gradientClip = 5.0;
    return value;
}

void buildModelAndInputs(
    const fs::path& root,
    CompositionInferencePipeline& pipeline,
    std::vector<double>& global,
    std::vector<double>& context)
{
    const auto trained =
        trainCompositionNeuralModelFromMidiCorpus(
            root.string(),
            false,
            0.20,
            0.20,
            config());

    expect(
        trained.isValid(),
        "real MIDI corpus trains before AI MIDI generation");

    pipeline =
        buildCompositionInferencePipeline(
            trained.neuralTraining.model);

    expect(
        pipeline.isValid(),
        "trained model creates valid inference pipeline");

    const auto& batch =
        trained.pipeline.prepared.normalizedBatch;

    expect(
        batch.sampleCount > 0 &&
        batch.maxSectionCount > 1,
        "prepared corpus contains generation context");

    global.assign(
        batch.globalMatrix.begin(),
        batch.globalMatrix.begin() +
            batch.globalFeatureWidth);

    context.assign(
        batch.sectionMatrix.begin(),
        batch.sectionMatrix.begin() +
            batch.sectionFeatureWidth);
}

midigengx::domain::MusicalContext buildContext()
{
    midigengx::domain::MusicalContext context;

    context.parameters.lengthBars = 16;
    context.parameters.phraseLengthBars = 4;

    context.normalize();

    return context;
}

void testGeneratedFeaturesBecomeRealMidiPhrases()
{
    const auto root =
        createCorpus(12);

    CompositionInferencePipeline pipeline;
    std::vector<double> global;
    std::vector<double> context;

    buildModelAndInputs(
        root,
        pipeline,
        global,
        context);

    const auto result =
        generateAIMidiPhrases(
            pipeline,
            global,
            context,
            buildContext(),
            4,
            77);

    expect(
        result.isValid(),
        "AI generated feature rollout becomes valid MIDI phrases");

    expect(
        result.phrases.size() == 4,
        "requested phrase count is preserved");

    for (const auto& phrase :
         result.phrases)
    {
        expect(
            phrase.isValid(),
            "every generated AI phrase is valid real MIDI note data");

        expect(
            !phrase.notes.empty(),
            "generated AI phrase contains notes");
    }

    cleanup(root);
}

void testGenerationIsDeterministic()
{
    const auto root =
        createCorpus(12);

    CompositionInferencePipeline pipeline;
    std::vector<double> global;
    std::vector<double> context;

    buildModelAndInputs(
        root,
        pipeline,
        global,
        context);

    const auto first =
        generateAIMidiPhrases(
            pipeline,
            global,
            context,
            buildContext(),
            3,
            123);

    const auto second =
        generateAIMidiPhrases(
            pipeline,
            global,
            context,
            buildContext(),
            3,
            123);

    expect(
        first.isValid() &&
        second.isValid(),
        "deterministic AI MIDI generation runs are valid");

    expect(
        first.phrases.size() ==
            second.phrases.size(),
        "deterministic AI MIDI generation preserves phrase count");

    for (std::size_t index = 0;
         index < first.phrases.size();
         ++index)
    {
        const auto& left =
            first.phrases[index];

        const auto& right =
            second.phrases[index];

        expect(
            left.notes.size() ==
                right.notes.size() &&
            left.lengthBeats ==
                right.lengthBeats,
            "deterministic AI MIDI phrase structure is stable");
    }

    cleanup(root);
}

void testUserConstraintsRemainAuthoritative()
{
    const auto root =
        createCorpus(12);

    CompositionInferencePipeline pipeline;
    std::vector<double> global;
    std::vector<double> context;

    buildModelAndInputs(
        root,
        pipeline,
        global,
        context);

    auto musicalContext =
        buildContext();

    musicalContext.parameters.octaveLow = -2;
    musicalContext.parameters.octaveHigh = 0;

    const auto result =
        generateAIMidiPhrases(
            pipeline,
            global,
            context,
            musicalContext,
            2,
            91);

    expect(
        result.isValid(),
        "constrained AI MIDI generation is valid");

    const auto internalLow =
        musicalContext.parameters.octaveLow;

    const auto internalHigh =
        musicalContext.parameters.octaveHigh;

    const int expectedLowMidi =
        12 * (internalLow + 1);

    const int expectedHighMidi =
        12 * (internalHigh + 1);

    for (const auto& phrase :
         result.phrases)
    {
        for (const auto& note :
             phrase.notes)
        {
            expect(
                note.midiNote >= expectedLowMidi &&
                note.midiNote <= expectedHighMidi,
                "hard octave constraint remains authoritative");
        }
    }

    cleanup(root);
}

void testInvalidInputsAreRejected()
{
    CompositionInferencePipeline pipeline;

    const auto result =
        generateAIMidiPhrases(
            pipeline,
            {},
            {},
            buildContext(),
            2,
            1);

    expect(
        !result.valid,
        "invalid AI MIDI generation input is rejected");
}

} // namespace

int main()
{
    testGeneratedFeaturesBecomeRealMidiPhrases();
    testGenerationIsDeterministic();
    testUserConstraintsRemainAuthoritative();
    testInvalidInputsAreRejected();

    std::cout
        << "MIDI-GenGX AI MIDI generation service tests passed.\n";

    return 0;
}
