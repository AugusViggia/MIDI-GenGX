#include "Music/CompositionMidiCorpusTrainingService.h"

#include <cmath>
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

void appendU32BE(std::vector<std::uint8_t>& b, std::uint32_t v)
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
        fs::temp_directory_path() / "MIDI_GenGX_Phase79_Corpus";

    std::error_code error;
    fs::remove_all(root, error);
    fs::create_directories(root, error);

    for (std::size_t i = 0; i < count; ++i)
    {
        const auto bytes =
            buildMidi(60 + static_cast<int>(i % 5));

        std::ofstream file(
            root / ("train-" + std::to_string(i) + ".mid"),
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

void testRealMidiCorpusTrains()
{
    const auto root = createCorpus(8);

    const auto result =
        trainCompositionNeuralModelFromMidiCorpus(
            root.string(),
            false,
            0.25,
            0.25,
            config());

    expect(result.isValid(),
           "real MIDI corpus produces a valid trained model");

    expect(result.sampleCount() == 8,
           "training result preserves corpus sample count");

    expect(result.neuralTraining.training.trained,
           "neural training reports trained");

    expect(result.neuralTraining.artifact.isValid(),
           "trained neural artifact is valid");

    cleanup(root);
}

void testTrainingProducesNonConstantLoss()
{
    const auto root = createCorpus(8);

    const auto result =
        trainCompositionNeuralModelFromMidiCorpus(
            root.string(),
            false,
            0.25,
            0.25,
            config());

    expect(result.isValid(),
           "trained corpus result is valid");

    expect(
        std::isfinite(result.neuralTraining.training.initialLoss) &&
        std::isfinite(result.neuralTraining.training.finalLoss),
        "training losses are finite");

    expect(
        std::fabs(
            result.neuralTraining.training.finalLoss -
            result.neuralTraining.training.initialLoss) >
            1.0e-12,
        "real-corpus training changes the loss");

    cleanup(root);
}

void testInvalidCorpusIsRejected()
{
    const auto root =
        fs::temp_directory_path() /
        "MIDI_GenGX_Phase79_Missing";

    std::error_code error;
    fs::remove_all(root, error);

    const auto result =
        trainCompositionNeuralModelFromMidiCorpus(
            root.string(),
            true,
            0.20,
            0.20,
            config());

    expect(!result.valid,
           "invalid MIDI corpus is rejected");

    expect(!result.neuralTraining.valid,
           "neural training does not run on invalid corpus");
}

void testTrainingIsDeterministic()
{
    const auto root = createCorpus(8);

    const auto first =
        trainCompositionNeuralModelFromMidiCorpus(
            root.string(),
            false,
            0.25,
            0.25,
            config());

    const auto second =
        trainCompositionNeuralModelFromMidiCorpus(
            root.string(),
            false,
            0.25,
            0.25,
            config());

    expect(first.isValid() && second.isValid(),
           "deterministic real-corpus training runs are valid");

    expect(
        first.neuralTraining.artifact.bytes ==
            second.neuralTraining.artifact.bytes,
        "real-corpus training artifact is deterministic");

    cleanup(root);
}

} // namespace

int main()
{
    testRealMidiCorpusTrains();
    testTrainingProducesNonConstantLoss();
    testInvalidCorpusIsRejected();
    testTrainingIsDeterministic();

    std::cout
        << "MIDI-GenGX real MIDI corpus training service tests passed.\n";
    return 0;
}
