#include "CompositionConditionedCorpusTrainingService.h"
#include "CompositionSequenceMetadataFileLoader.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

using namespace midigengx::music;

namespace
{

void printUsage()
{
    std::cout
        << "MIDI-GenGX real composer training\n"
        << "\n"
        << "Required:\n"
        << "  --midi-dir <directory>\n"
        << "  --metadata <tsv-file>\n"
        << "  --output-model <file>\n"
        << "\n"
        << "Optional:\n"
        << "  --epochs <count>           default 25\n"
        << "  --learning-rate <value>    default 0.001\n"
        << "  --gradient-clip <value>    default 1.0\n"
        << "  --non-recursive\n";
}

bool parsePositiveDouble(
    const std::string& value,
    double& output)
{
    char* end = nullptr;

    const auto parsed =
        std::strtod(
            value.c_str(),
            &end);

    if (end == nullptr ||
        *end != '\0' ||
        parsed <= 0.0)
    {
        return false;
    }

    output = parsed;
    return true;
}

bool parsePositiveSize(
    const std::string& value,
    std::size_t& output)
{
    char* end = nullptr;

    const auto parsed =
        std::strtoull(
            value.c_str(),
            &end,
            10);

    if (end == nullptr ||
        *end != '\0' ||
        parsed == 0)
    {
        return false;
    }

    output =
        static_cast<std::size_t>(
            parsed);

    return true;
}

bool writeBinary(
    const std::string& filePath,
    const std::vector<std::uint8_t>& bytes)
{
    std::ofstream file(
        filePath,
        std::ios::binary);

    if (!file)
        return false;

    file.write(
        reinterpret_cast<const char*>(
            bytes.data()),
        static_cast<std::streamsize>(
            bytes.size()));

    return static_cast<bool>(
        file);
}

} // namespace

int main(int argc, char** argv)
{
    if (argc < 7)
    {
        printUsage();
        return 2;
    }

    std::string midiDirectory;
    std::string metadataPath;
    std::string outputModelPath;

    std::size_t epochs = 25;
    double learningRate = 0.001;
    double gradientClip = 1.0;
    bool recursive = true;

    for (int index = 1;
         index < argc;
         ++index)
    {
        const std::string argument =
            argv[index];

        if (argument == "--midi-dir" &&
            index + 1 < argc)
        {
            midiDirectory =
                argv[++index];
        }
        else if (argument == "--metadata" &&
                 index + 1 < argc)
        {
            metadataPath =
                argv[++index];
        }
        else if (argument == "--output-model" &&
                 index + 1 < argc)
        {
            outputModelPath =
                argv[++index];
        }
        else if (argument == "--epochs" &&
                 index + 1 < argc)
        {
            if (!parsePositiveSize(
                    argv[++index],
                    epochs))
            {
                std::cerr
                    << "Invalid --epochs value.\n";
                return 2;
            }
        }
        else if (argument == "--learning-rate" &&
                 index + 1 < argc)
        {
            if (!parsePositiveDouble(
                    argv[++index],
                    learningRate))
            {
                std::cerr
                    << "Invalid --learning-rate value.\n";
                return 2;
            }
        }
        else if (argument == "--gradient-clip" &&
                 index + 1 < argc)
        {
            if (!parsePositiveDouble(
                    argv[++index],
                    gradientClip))
            {
                std::cerr
                    << "Invalid --gradient-clip value.\n";
                return 2;
            }
        }
        else if (argument == "--non-recursive")
        {
            recursive = false;
        }
        else
        {
            printUsage();
            return 2;
        }
    }

    const auto metadata =
        loadCompositionSequenceMetadataFile(
            metadataPath);

    if (!metadata.isValid())
    {
        std::cerr
            << "Metadata file is invalid. Parsed="
            << metadata.parsedEntryCount
            << " rejected="
            << metadata.rejectedLineCount
            << '\n';

        return 3;
    }

    CompositionConditionedSequenceNeuralTrainingConfig config;

    config.epochs =
        epochs;

    config.learningRate =
        learningRate;

    config.gradientClip =
        gradientClip;

    const auto result =
        runCompositionConditionedCorpusTraining(
            midiDirectory,
            recursive,
            metadata.catalog,
            config);

    if (!result.isValid())
    {
        std::cerr
            << "Real composer training failed.\n";

        std::cerr
            << "input="
            << result.inputFileCount
            << " sequences="
            << result.sequenceCount
            << " rejected="
            << result.rejectedFileCount
            << " train="
            << result.trainingSampleCount
            << " validation="
            << result.validationSampleCount
            << " test="
            << result.testSampleCount
            << '\n';

        return 4;
    }

    if (!writeBinary(
            outputModelPath,
            result.trainingRun.modelArtifact.bytes))
    {
        std::cerr
            << "Could not write model artifact: "
            << outputModelPath
            << '\n';

        return 5;
    }

    std::cout
        << "REAL COMPOSER TRAINING COMPLETE\n"
        << "input=" << result.inputFileCount << '\n'
        << "sequences=" << result.sequenceCount << '\n'
        << "rejected=" << result.rejectedFileCount << '\n'
        << "train=" << result.trainingSampleCount << '\n'
        << "validation=" << result.validationSampleCount << '\n'
        << "test=" << result.testSampleCount << '\n'
        << "epochs=" << result.trainingRun.training.epochsCompleted << '\n'
        << "initialLoss=" << result.trainingRun.training.initialLoss << '\n'
        << "finalLoss=" << result.trainingRun.training.finalLoss << '\n'
        << "validationLoss="
        << (result.validationEvaluation.valid
            ? result.validationEvaluation.loss
            : -1.0)
        << '\n'
        << "testLoss="
        << (result.testEvaluation.valid
            ? result.testEvaluation.loss
            : -1.0)
        << '\n'
        << "model=" << outputModelPath << '\n';

    return 0;
}
