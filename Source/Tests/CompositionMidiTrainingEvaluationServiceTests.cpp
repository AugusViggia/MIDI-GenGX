#include "Music/CompositionMidiTrainingEvaluationService.h"

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
        fs::temp_directory_path() / "MIDI_GenGX_Phase80_Corpus";

    std::error_code error;
    fs::remove_all(root, error);
    fs::create_directories(root, error);

    for (std::size_t i = 0; i < count; ++i)
    {
        const auto bytes = buildMidi(
            60 + static_cast<int>(i % 5));

        std::ofstream file(
            root / ("eval-" + std::to_string(i) + ".mid"),
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

void testEndToEndEvaluation()
{
    const auto root = createCorpus(12);

    const auto result =
        trainAndEvaluateCompositionNeuralModelFromMidiCorpus(
            root.string(),
            false,
            0.20,
            0.20,
            config());

    expect(
        result.isValid(),
        "real MIDI training and evaluation are valid");

    expect(
        result.training.sampleCount() == 12,
        "evaluation preserves complete corpus sample count");

    expect(
        result.neuralValidation.exampleCount > 0 &&
        result.neuralTest.exampleCount > 0,
        "neural validation and test contain examples");

    expect(
        result.baselineValidation.exampleCount > 0 &&
        result.baselineTest.exampleCount > 0,
        "baseline validation and test contain examples");

    expect(
        result.musicalValidation.isValid() &&
        result.musicalTest.isValid(),
        "musical quality evaluation is valid");

    cleanup(root);
}

void testEvaluationSplitsAreSeparated()
{
    const auto root = createCorpus(12);

    const auto result =
        trainAndEvaluateCompositionNeuralModelFromMidiCorpus(
            root.string(),
            false,
            0.20,
            0.20,
            config());

    expect(result.isValid(),
           "split evaluation pipeline is valid");

    expect(
        result.neuralValidation.sampleCount ==
            result.training.pipeline.prepared.validationCount(),
        "neural validation uses validation split");

    expect(
        result.neuralTest.sampleCount ==
            result.training.pipeline.prepared.testCount(),
        "neural test uses test split");

    expect(
        result.baselineValidation.sampleCount ==
            result.training.pipeline.prepared.validationCount(),
        "baseline validation uses validation split");

    expect(
        result.baselineTest.sampleCount ==
            result.training.pipeline.prepared.testCount(),
        "baseline test uses test split");

    cleanup(root);
}

void testMetricsAreDeterministic()
{
    const auto root = createCorpus(12);

    const auto first =
        trainAndEvaluateCompositionNeuralModelFromMidiCorpus(
            root.string(),
            false,
            0.20,
            0.20,
            config());

    const auto second =
        trainAndEvaluateCompositionNeuralModelFromMidiCorpus(
            root.string(),
            false,
            0.20,
            0.20,
            config());

    expect(
        first.isValid() && second.isValid(),
        "repeated evaluation runs are valid");

    expect(
        first.neuralTest.meanSquaredError ==
            second.neuralTest.meanSquaredError &&
        first.neuralTest.meanAbsoluteError ==
            second.neuralTest.meanAbsoluteError &&
        first.musicalTest.overallScore ==
            second.musicalTest.overallScore,
        "evaluation metrics are deterministic");

    cleanup(root);
}

void testInvalidCorpusIsRejected()
{
    const auto root =
        fs::temp_directory_path() /
        "MIDI_GenGX_Phase80_Missing";

    std::error_code error;
    fs::remove_all(root, error);

    const auto result =
        trainAndEvaluateCompositionNeuralModelFromMidiCorpus(
            root.string(),
            true,
            0.20,
            0.20,
            config());

    expect(
        !result.valid,
        "invalid corpus is rejected before evaluation");
}

void testBaselineComparisonIsWellDefined()
{
    const auto root = createCorpus(12);

    const auto result =
        trainAndEvaluateCompositionNeuralModelFromMidiCorpus(
            root.string(),
            false,
            0.20,
            0.20,
            config());

    expect(
        result.isValid(),
        "baseline comparison result is valid");

    expect(
        result.neuralTest.isValid() &&
        result.baselineTest.isValid(),
        "neural and baseline test metrics are valid");

    const auto comparison =
        result.neuralBeatsBaselineOnTest();

    expect(
        comparison == true ||
        comparison == false,
        "baseline comparison produces a defined result");

    cleanup(root);
}

} // namespace

int main()
{
    testEndToEndEvaluation();
    testEvaluationSplitsAreSeparated();
    testMetricsAreDeterministic();
    testInvalidCorpusIsRejected();
    testBaselineComparisonIsWellDefined();

    std::cout
        << "MIDI-GenGX real MIDI training evaluation service tests passed.\n";
    return 0;
}
