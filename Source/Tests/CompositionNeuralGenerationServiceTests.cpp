#include "Music/CompositionMidiCorpusTrainingService.h"
#include "Music/CompositionNeuralGenerationService.h"

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
        std::cerr << "FAILED: " << message << '\n';
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
            track.push_back(static_cast<std::uint8_t>(root + chord[note]));
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
            track.push_back(static_cast<std::uint8_t>(root + chord[note]));
            track.push_back(0);
        }
    }

    track.push_back(0x00);
    track.push_back(0xFF);
    track.push_back(0x2F);
    track.push_back(0x00);

    bytes.push_back('M'); bytes.push_back('T');
    bytes.push_back('r'); bytes.push_back('k');
    appendU32BE(bytes, static_cast<std::uint32_t>(track.size()));
    bytes.insert(bytes.end(), track.begin(), track.end());
    return bytes;
}

fs::path createCorpus(std::size_t count)
{
    const auto root =
        fs::temp_directory_path() / "MIDI_GenGX_Phase81_Corpus";

    std::error_code error;
    fs::remove_all(root, error);
    fs::create_directories(root, error);

    for (std::size_t i = 0; i < count; ++i)
    {
        const auto bytes =
            buildMidi(60 + static_cast<int>(i % 5));

        std::ofstream file(
            root / ("generation-" + std::to_string(i) + ".mid"),
            std::ios::binary);

        file.write(
            reinterpret_cast<const char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
    }

    return root;
}

void cleanup(const fs::path& root)
{
    std::error_code error;
    fs::remove_all(root, error);
}

CompositionNeuralTrainingConfig config()
{
    CompositionNeuralTrainingConfig value;
    value.epochs = 8;
    value.learningRate = 0.001;
    value.optimizer = CompositionNeuralOptimizer::Adam;
    value.gradientClip = 5.0;
    return value;
}

void testAutoregressiveRollout()
{
    const auto root =
        createCorpus(12);

    const auto trained =
        trainCompositionNeuralModelFromMidiCorpus(
            root.string(),
            false,
            0.20,
            0.20,
            config());

    expect(
        trained.isValid(),
        "real MIDI corpus trains before generation");

    const auto pipeline =
        buildCompositionInferencePipeline(
            trained.neuralTraining.model);

    expect(
        pipeline.isValid(),
        "trained model creates a valid inference pipeline");

    const auto& prepared =
        trained.pipeline.prepared.normalizedBatch;

    expect(
        prepared.sampleCount > 0 &&
        prepared.maxSectionCount > 1,
        "prepared corpus contains section context");

    const std::size_t sampleIndex = 0;

    const auto globalBase =
        sampleIndex *
        prepared.globalFeatureWidth;

    const auto contextBase =
        sampleIndex *
        prepared.sectionFeatureWidth;

    std::vector<double> global(
        prepared.globalMatrix.begin() +
            globalBase,
        prepared.globalMatrix.begin() +
            globalBase +
            prepared.globalFeatureWidth);

    std::vector<double> context(
        prepared.sectionMatrix.begin() +
            contextBase,
        prepared.sectionMatrix.begin() +
            contextBase +
            prepared.sectionFeatureWidth);

    const auto generated =
        generateNextSections(
            pipeline,
            global,
            context,
            6);

    expect(
        generated.isValid(
            pipeline.contract.targetWidth),
        "autoregressive neural generation is valid");

    expect(
        generated.generatedSections.size() == 6,
        "generation returns the requested number of sections");

    cleanup(root);
}

void testGenerationIsDeterministic()
{
    const auto root =
        createCorpus(12);

    const auto trained =
        trainCompositionNeuralModelFromMidiCorpus(
            root.string(),
            false,
            0.20,
            0.20,
            config());

    expect(
        trained.isValid(),
        "deterministic generation training is valid");

    const auto pipeline =
        buildCompositionInferencePipeline(
            trained.neuralTraining.model);

    const auto& prepared =
        trained.pipeline.prepared.normalizedBatch;

    const std::size_t sampleIndex = 0;

    const auto globalBase =
        sampleIndex *
        prepared.globalFeatureWidth;

    const auto contextBase =
        sampleIndex *
        prepared.sectionFeatureWidth;

    std::vector<double> global(
        prepared.globalMatrix.begin() +
            globalBase,
        prepared.globalMatrix.begin() +
            globalBase +
            prepared.globalFeatureWidth);

    std::vector<double> context(
        prepared.sectionMatrix.begin() +
            contextBase,
        prepared.sectionMatrix.begin() +
            contextBase +
            prepared.sectionFeatureWidth);

    const auto first =
        generateNextSections(
            pipeline,
            global,
            context,
            5);

    const auto second =
        generateNextSections(
            pipeline,
            global,
            context,
            5);

    expect(
        first.isValid(
            pipeline.contract.targetWidth) &&
        second.isValid(
            pipeline.contract.targetWidth),
        "deterministic generation runs are valid");

    expect(
        first.generatedSections ==
            second.generatedSections,
        "identical model and seed context produce identical generation");

    cleanup(root);
}

void testGenerationChangesContext()
{
    const auto root =
        createCorpus(12);

    const auto trained =
        trainCompositionNeuralModelFromMidiCorpus(
            root.string(),
            false,
            0.20,
            0.20,
            config());

    expect(
        trained.isValid(),
        "context rollout training is valid");

    const auto pipeline =
        buildCompositionInferencePipeline(
            trained.neuralTraining.model);

    const auto& prepared =
        trained.pipeline.prepared.normalizedBatch;

    std::vector<double> global(
        prepared.globalMatrix.begin(),
        prepared.globalMatrix.begin() +
            prepared.globalFeatureWidth);

    std::vector<double> context(
        prepared.sectionMatrix.begin(),
        prepared.sectionMatrix.begin() +
            prepared.sectionFeatureWidth);

    const auto generated =
        generateNextSections(
            pipeline,
            global,
            context,
            4);

    expect(
        generated.isValid(
            pipeline.contract.targetWidth),
        "context rollout result is valid");

    expect(
        generated.generatedSections.front() != context,
        "neural generation produces a prediction rather than returning the seed context unchanged");

    cleanup(root);
}

void testInvalidInputsAreRejected()
{
    CompositionInferencePipeline invalid;

    const auto generated =
        generateNextSections(
            invalid,
            {},
            {},
            4);

    expect(
        !generated.valid,
        "invalid generation pipeline is rejected");
}

} // namespace

int main()
{
    testAutoregressiveRollout();
    testGenerationIsDeterministic();
    testGenerationChangesContext();
    testInvalidInputsAreRejected();

    std::cout
        << "MIDI-GenGX neural generation service tests passed.\n";
    return 0;
}
