#include "Music/CompositionConditionedSequenceNeuralModelRuntimeLoader.h"
#include "Music/CompositionConditionedSequenceNeuralEvaluator.h"
#include "Music/CompositionRealComposerCorpusPreparation.h"
#include "Music/CompositionComposerKnowledgeSample.h"
#include "Music/CompositionComposerKnowledgeCatalog.h"
#include "Music/CompositionComposerKnowledgePartition.h"
#include "Music/CompositionSequenceMetadataFileLoader.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

using namespace midigengx::music;

namespace
{
std::vector<std::uint8_t> readBinary(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
        return {};

    return std::vector<std::uint8_t>(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>());
}

struct SplitDatasets
{
    CompositionConditionedTrainingDataset validation;
    CompositionConditionedTrainingDataset test;
    bool valid = false;
};

SplitDatasets buildSplits(
    const CompositionRealComposerCorpusPreparationResult& prepared,
    const CompositionSequenceMetadataCatalog& metadata)
{
    SplitDatasets result;

    std::vector<CompositionComposerKnowledgeSample> knowledgeSamples;
    knowledgeSamples.reserve(prepared.sequences.size());

    for (const auto& sequence : prepared.sequences)
    {
        const auto* entry =
            metadata.findBySampleId(sequence.sampleId);
        if (entry == nullptr)
            return result;

        CompositionDatasetSample composition;
        composition.sampleId = sequence.sampleId;
        composition.globalFeatures = {0.0, 0.0, 0.0};
        composition.analysisValid = true;

        const auto knowledge =
            buildCompositionComposerKnowledgeSample(
                composition,
                *entry);

        if (!knowledge.isValid())
            return result;

        knowledgeSamples.push_back(knowledge);
    }

    const auto catalog =
        buildCompositionComposerKnowledgeCatalog(knowledgeSamples);
    if (!catalog.isValid())
        return result;

    const auto partition =
        buildCompositionComposerKnowledgePartition(
            catalog, 0.2, 0.1);

    if (!partition.isValid(catalog.sampleCount()) ||
        partition.trainingCount() != 48 ||
        partition.validationCount() != 13 ||
        partition.testCount() != 6)
    {
        return result;
    }

    std::vector<CompositionComposerKnowledgeSample> flattened;
    for (const auto& composer : catalog.composers)
        flattened.insert(
            flattened.end(),
            composer.samples.begin(),
            composer.samples.end());

    const auto buildSplit =
        [&](const std::vector<std::size_t>& indices)
        {
            std::vector<CompositionMidiTrainingSequence> sequences;
            sequences.reserve(indices.size());

            for (const auto index : indices)
            {
                if (index >= flattened.size())
                    return std::vector<CompositionMidiTrainingSequence>{};

                const auto& target = flattened[index];
                bool found = false;

                for (const auto& sequence : prepared.sequences)
                {
                    if (sequence.sampleId ==
                        target.composition.sampleId)
                    {
                        sequences.push_back(sequence);
                        found = true;
                        break;
                    }
                }

                if (!found)
                    return std::vector<CompositionMidiTrainingSequence>{};
            }

            return sequences;
        };

    const auto validationSequences =
        buildSplit(partition.validationSampleIndices);
    const auto testSequences =
        buildSplit(partition.testSampleIndices);

    if (validationSequences.size() != 13 ||
        testSequences.size() != 6)
        return result;

    result.validation =
        buildCompositionConditionedTrainingDataset(
            validationSequences,
            metadata);

    result.test =
        buildCompositionConditionedTrainingDataset(
            testSequences,
            metadata);

    result.valid =
        result.validation.isValid() &&
        result.test.isValid() &&
        result.validation.sampleCount() == 13 &&
        result.test.sampleCount() == 6;

    return result;
}

bool evaluate(
    const std::string& label,
    const std::string& path,
    const SplitDatasets& splits,
    std::ostream& output)
{
    const auto bytes = readBinary(path);
    if (bytes.empty())
    {
        output << label << ".fileValid=0\n";
        return false;
    }

    const auto loaded =
        CompositionConditionedSequenceNeuralModelRuntimeLoader{}.load(bytes);

    if (!loaded.isValid())
    {
        output << label << ".modelLoaded=0\n";
        return false;
    }

    const auto validation =
        evaluateCompositionConditionedSequenceNeuralModel(
            loaded.model,
            splits.validation);

    const auto test =
        evaluateCompositionConditionedSequenceNeuralModel(
            loaded.model,
            splits.test);

    if (!validation.isValid() || !test.isValid())
    {
        output << label << ".evaluationValid=0\n";
        return false;
    }

    output
        << label << ".fileValid=1\n"
        << label << ".modelLoaded=1\n"
        << label << ".validationSamples="
        << validation.sampleCount << '\n'
        << label << ".validationWindows="
        << validation.windowCount << '\n'
        << label << ".validationLoss="
        << std::setprecision(12)
        << validation.loss << '\n'
        << label << ".testSamples="
        << test.sampleCount << '\n'
        << label << ".testWindows="
        << test.windowCount << '\n'
        << label << ".testLoss="
        << std::setprecision(12)
        << test.loss << '\n';

    return true;
}
}

int main(int argc, char* argv[])
{
    if (argc != 5)
    {
        std::cerr
            << "FAILED: usage: "
            << "Phase121ValidationTestComparison "
            << "<corpus> <metadata.tsv> "
            << "<phase119.mgcn> <phase121.mgcn>\n";
        return 1;
    }

    const std::string corpus = argv[1];
    const std::string metadataPath = argv[2];
    const std::string phase119Path = argv[3];
    const std::string phase121Path = argv[4];

    const auto metadata =
        loadCompositionSequenceMetadataFile(metadataPath);

    if (!metadata.isValid())
    {
        std::cerr << "FAILED: metadata invalid\n";
        return 1;
    }

    const auto prepared =
        prepareRealComposerCorpusFromDirectory(
            corpus,
            true,
            metadata.catalog);

    if (!prepared.isValid() ||
        prepared.sequences.size() != 67)
    {
        std::cerr << "FAILED: expected 67 prepared sequences\n";
        return 1;
    }

    const auto splits =
        buildSplits(prepared, metadata.catalog);

    if (!splits.valid)
    {
        std::cerr << "FAILED: 48/13/6 partition invalid\n";
        return 1;
    }

    std::ofstream report(
        "phase121-validation-test-comparison.txt");

    if (!report)
    {
        std::cerr
            << "FAILED: could not create report\n";
        return 1;
    }

    report
        << "PHASE 121 VALIDATION TEST COMPARISON\n"
        << "trainingSamples=48\n"
        << "validationSamples=13\n"
        << "testSamples=6\n";

    if (!evaluate(
            "phase119",
            phase119Path,
            splits,
            report) ||
        !evaluate(
            "phase121",
            phase121Path,
            splits,
            report))
    {
        return 1;
    }

    report
        << "evaluationComplete=1\n";

    report.flush();

    std::ifstream display(
        "phase121-validation-test-comparison.txt");

    std::cout << display.rdbuf();
    return 0;
}
