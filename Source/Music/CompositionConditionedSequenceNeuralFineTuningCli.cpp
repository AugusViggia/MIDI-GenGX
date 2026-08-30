#include "Music/CompositionConditionedSequenceNeuralModelRuntimeLoader.h"
#include "Music/CompositionConditionedSequenceNeuralTrainer.h"
#include "Music/CompositionConditionedSequenceNeuralModelArtifact.h"
#include "Music/CompositionRealComposerCorpusPreparation.h"
#include "Music/CompositionComposerKnowledgeSample.h"
#include "Music/CompositionComposerKnowledgeCatalog.h"
#include "Music/CompositionComposerKnowledgePartition.h"
#include "Music/CompositionDatasetSample.h"
#include "Music/CompositionSequenceMetadataFileLoader.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

using namespace midigengx::music;

namespace
{

void fail(const std::string& message)
{
    std::cerr << "FAILED: " << message << '\n';
    std::exit(1);
}

std::vector<std::uint8_t>
readBinaryFile(
    const std::string& path)
{
    std::ifstream file(
        path,
        std::ios::binary);

    if (!file)
        return {};

    return std::vector<std::uint8_t>(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
}

std::vector<CompositionMidiTrainingSequence>
buildTrainingSequences(
    const CompositionRealComposerCorpusPreparationResult& prepared)
{
    std::vector<CompositionComposerKnowledgeSample>
        knowledgeSamples;

    knowledgeSamples.reserve(
        prepared.sequences.size());

    for (const auto& sequence :
         prepared.sequences)
    {
        const auto* metadata =
            prepared.metadataCatalog.findBySampleId(
                sequence.sampleId);

        if (metadata == nullptr)
            return {};

        CompositionDatasetSample composition;
        composition.sampleId =
            sequence.sampleId;
        composition.globalFeatures =
        {
            0.0,
            0.0,
            0.0
        };
        composition.analysisValid = true;

        const auto knowledge =
            buildCompositionComposerKnowledgeSample(
                composition,
                *metadata);

        if (!knowledge.isValid())
            return {};

        knowledgeSamples.push_back(
            knowledge);
    }

    const auto catalog =
        buildCompositionComposerKnowledgeCatalog(
            knowledgeSamples);

    if (!catalog.isValid())
        return {};

    const auto partition =
        buildCompositionComposerKnowledgePartition(
            catalog,
            0.2,
            0.1);

    if (!partition.isValid(
            catalog.sampleCount()))
    {
        return {};
    }

    if (partition.trainingCount() != 48 ||
        partition.validationCount() != 13 ||
        partition.testCount() != 6)
    {
        return {};
    }

    std::vector<CompositionComposerKnowledgeSample>
        flattened;

    for (const auto& composer :
         catalog.composers)
    {
        flattened.insert(
            flattened.end(),
            composer.samples.begin(),
            composer.samples.end());
    }

    std::vector<CompositionMidiTrainingSequence>
        trainingSequences;

    trainingSequences.reserve(
        partition.trainingSampleIndices.size());

    for (const auto index :
         partition.trainingSampleIndices)
    {
        if (index >= flattened.size())
            return {};

        const auto& targetSample =
            flattened[index];

        bool found = false;

        for (const auto& sequence :
             prepared.sequences)
        {
            if (sequence.sampleId ==
                targetSample.composition.sampleId)
            {
                trainingSequences.push_back(
                    sequence);
                found = true;
                break;
            }
        }

        if (!found)
            return {};
    }

    if (trainingSequences.size() != 48)
        return {};

    return trainingSequences;
}

bool sameVocabulary(
    const CompositionConditioningVocabulary& left,
    const CompositionConditioningVocabulary& right)
{
    return left.composers == right.composers &&
           left.styles == right.styles &&
           left.eras == right.eras &&
           left.instrumentations == right.instrumentations;
}

} // namespace

int main(int argc, char* argv[])
{
    if (argc < 5 || argc > 8)
    {
        fail(
            "usage: "
            "CompositionConditionedSequenceNeuralFineTuningCli "
            "<midi-directory> <metadata.tsv> <base-model.mgcn> "
            "<output-model.mgcn> [epochs] [learning-rate] [workers]");
    }

    const std::string midiDirectory = argv[1];
    const std::string metadataPath = argv[2];
    const std::string baseModelPath = argv[3];
    const std::string outputModelPath = argv[4];

    std::size_t epochs = 25;
    double learningRate = 0.00025;
    constexpr double gradientClip = 1.0;
    std::size_t workers = 8;

    if (argc >= 6)
    {
        const auto parsed =
            std::strtoull(
                argv[5],
                nullptr,
                10);

        if (parsed == 0)
            fail("epochs must be greater than zero");

        epochs =
            static_cast<std::size_t>(
                parsed);
    }

    if (argc >= 7)
    {
        learningRate =
            std::strtod(
                argv[6],
                nullptr);

        if (!std::isfinite(learningRate) ||
            learningRate <= 0.0)
        {
            fail("learning-rate must be finite and positive");
        }
    }

    if (argc >= 8)
    {
        const auto parsedWorkers =
            std::strtoull(
                argv[7],
                nullptr,
                10);

        if (parsedWorkers == 0)
            fail("workers must be greater than zero");

        workers =
            static_cast<std::size_t>(
                parsedWorkers);
    }

    const auto metadata =
        loadCompositionSequenceMetadataFile(
            metadataPath);

    if (!metadata.isValid())
        fail("metadata catalog is invalid");

    const auto prepared =
        prepareRealComposerCorpusFromDirectory(
            midiDirectory,
            true,
            metadata.catalog);

    if (!prepared.isValid() ||
        prepared.inputSampleCount != 67 ||
        prepared.acceptedSampleCount != 67 ||
        prepared.rejectedSampleCount != 0 ||
        prepared.sequences.size() != 67 ||
        prepared.conditionedDataset.sampleCount() != 67)
    {
        fail("real Chopin corpus acceptance changed unexpectedly");
    }

    const auto baseBytes =
        readBinaryFile(
            baseModelPath);

    if (baseBytes.empty())
        fail("base model is missing or empty");

    const auto loaded =
        CompositionConditionedSequenceNeuralModelRuntimeLoader{}
            .load(baseBytes);

    if (!loaded.isValid())
        fail("base model failed runtime loading");

    const auto trainingSequences =
        buildTrainingSequences(
            prepared);

    if (trainingSequences.size() != 48)
        fail("training split is not exactly 48 samples");

    const auto trainingDataset =
        buildCompositionConditionedTrainingDataset(
            trainingSequences,
            prepared.metadataCatalog);

    if (!trainingDataset.isValid() ||
        trainingDataset.sampleCount() != 48)
    {
        fail("fine-tuning dataset is invalid");
    }

    if (!sameVocabulary(
            loaded.model.vocabulary,
            trainingDataset.vocabulary))
    {
        fail("base model vocabulary does not match fine-tuning dataset");
    }

    auto model =
        loaded.model;

    if (!model.isValid())
        fail("base model is invalid before fine-tuning");

    CompositionConditionedSequenceNeuralTrainingConfig config;
    config.epochs = epochs;
    config.learningRate = learningRate;
    config.gradientClip = gradientClip;
    config.workerThreads = workers;
    config.evaluationInterval = 1;

    config.progressCallback =
        [](const std::size_t epoch,
           const std::size_t totalEpochs,
           const double loss)
        {
            std::cout
                << "progress epoch="
                << epoch
                << "/"
                << totalEpochs;

            if (std::isfinite(loss))
                std::cout
                    << " loss="
                    << loss;

            std::cout
                << '\n';
        };

    std::cout
        << "PHASE 121B OPTIMIZED FINE-TUNING START\n"
        << "workerThreads="
        << config.workerThreads
        << '\n';

    const auto training =
        trainCompositionConditionedSequenceNeuralModel(
            model,
            trainingDataset,
            config);

    if (!training.isValid())
        fail("fine-tuning did not complete successfully");

    const auto artifact =
        serializeCompositionConditionedSequenceNeuralModel(
            model);

    if (!artifact.isValid())
        fail("fine-tuned model artifact is invalid");

    std::ofstream output(
        outputModelPath,
        std::ios::binary);

    if (!output)
        fail("output model cannot be created");

    output.write(
        reinterpret_cast<const char*>(
            artifact.bytes.data()),
        static_cast<std::streamsize>(
            artifact.bytes.size()));

    if (!output)
        fail("output model write failed");

    std::cout
        << "PHASE 121B OPTIMIZED FINE-TUNING COMPLETE\n"
        << "baseModelLoaded=1\n"
        << "corpusSequences=67\n"
        << "trainingSamples=48\n"
        << "validationSamples=13\n"
        << "testSamples=6\n"
        << "fineTuneEpochs=" << training.epochsCompleted << '\n'
        << "learningRate=" << config.learningRate << '\n'
        << "gradientClip=" << config.gradientClip << '\n'
        << "workerThreads=" << config.workerThreads << '\n'
        << "windowCount=" << training.windowCount << '\n'
        << "initialLoss=" << training.initialLoss << '\n'
        << "finalLoss=" << training.finalLoss << '\n'
        << "modelBytes=" << artifact.bytes.size() << '\n'
        << "outputModel=" << outputModelPath << '\n';

    return 0;
}
