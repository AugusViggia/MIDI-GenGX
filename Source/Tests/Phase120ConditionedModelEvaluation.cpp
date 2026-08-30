#include "Music/CompositionConditionedSequenceNeuralEvaluator.h"
#include "Music/CompositionConditionedSequenceNeuralModelArtifact.h"
#include "Music/CompositionComposerKnowledgeCatalog.h"
#include "Music/CompositionComposerKnowledgePartition.h"
#include "Music/CompositionConditionedTrainingDataset.h"
#include "Music/CompositionRealComposerCorpusPreparation.h"
#include "Music/CompositionSequenceMetadataFileLoader.h"
#include "Music/CompositionComposerKnowledgeSample.h"

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
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

bool sameVector(
    const std::vector<std::string>& left,
    const std::vector<std::string>& right)
{
    return left == right;
}

CompositionComposerKnowledgeCatalog
buildKnowledgeCatalog(
    const std::vector<CompositionMidiTrainingSequence>& sequences,
    const CompositionSequenceMetadataCatalog& metadataCatalog)
{
    std::vector<CompositionComposerKnowledgeSample> samples;
    samples.reserve(sequences.size());

    for (const auto& sequence : sequences)
    {
        const auto* metadata =
            metadataCatalog.findBySampleId(sequence.sampleId);

        if (metadata == nullptr)
            return {};

        CompositionDatasetSample composition;
        composition.sampleId = sequence.sampleId;
        composition.globalFeatures = { 0.0, 0.0, 0.0 };
        composition.analysisValid = true;

        const auto knowledge =
            buildCompositionComposerKnowledgeSample(
                composition,
                *metadata);

        if (!knowledge.isValid())
            return {};

        samples.push_back(knowledge);
    }

    return buildCompositionComposerKnowledgeCatalog(
        samples,
        true);
}

std::vector<CompositionMidiTrainingSequence>
selectSequences(
    const std::vector<CompositionMidiTrainingSequence>& sequences,
    const CompositionComposerKnowledgeCatalog& catalog,
    const std::vector<std::size_t>& indices)
{
    std::vector<CompositionComposerKnowledgeSample> flattened;

    for (const auto& composer : catalog.composers)
        flattened.insert(
            flattened.end(),
            composer.samples.begin(),
            composer.samples.end());

    std::vector<CompositionMidiTrainingSequence> selected;
    selected.reserve(indices.size());

    for (const auto index : indices)
    {
        if (index >= flattened.size())
            return {};

        const auto& sample = flattened[index];

        for (const auto& sequence : sequences)
        {
            if (sequence.sampleId ==
                sample.composition.sampleId)
            {
                selected.push_back(sequence);
                break;
            }
        }
    }

    if (selected.size() != indices.size())
        return {};

    return selected;
}

CompositionConditionedTrainingDataset
buildSplitDataset(
    const std::vector<CompositionMidiTrainingSequence>& sequences,
    const CompositionSequenceMetadataCatalog& metadataCatalog,
    const CompositionComposerKnowledgeCatalog& knowledgeCatalog,
    const std::vector<std::size_t>& indices)
{
    const auto splitSequences =
        selectSequences(
            sequences,
            knowledgeCatalog,
            indices);

    if (splitSequences.empty() && !indices.empty())
        return {};

    return buildCompositionConditionedTrainingDataset(
        splitSequences,
        metadataCatalog);
}

void printEvaluation(
    const char* name,
    const CompositionConditionedSequenceNeuralEvaluationResult& result)
{
    if (!result.isValid())
        fail(std::string(name) + " evaluation is invalid");

    std::cout
        << name
        << ".sampleCount=" << result.sampleCount << '\n'
        << name
        << ".windowCount=" << result.windowCount << '\n'
        << name
        << ".loss=" << result.loss << '\n';
}

} // namespace

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        fail(
            "usage: Phase120ConditionedModelEvaluation "
            "<midi-directory> <metadata.tsv> <model.mgcn>");
    }

    const std::string midiDirectory = argv[1];
    const std::string metadataPath = argv[2];
    const std::string modelPath = argv[3];

    const auto metadataLoad =
        loadCompositionSequenceMetadataFile(
            metadataPath);

    if (!metadataLoad.isValid())
        fail("metadata catalog is invalid");

    const auto prepared =
        prepareRealComposerCorpusFromDirectory(
            midiDirectory,
            true,
            metadataLoad.catalog);

    if (!prepared.isValid())
        fail("real composer corpus preparation is invalid");

    if (prepared.inputSampleCount != 67 ||
        prepared.acceptedSampleCount != 67 ||
        prepared.rejectedSampleCount != 0 ||
        prepared.sequences.size() != 67)
    {
        fail("Phase 119 corpus acceptance changed unexpectedly");
    }

    std::ifstream file(
        modelPath,
        std::ios::binary);

    if (!file)
        fail("model file cannot be opened");

    const std::vector<std::uint8_t> bytes(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    CompositionConditionedSequenceNeuralModelArtifact artifact;
    artifact.bytes = bytes;

    if (!artifact.isValid())
        fail("model artifact is invalid");

    CompositionConditionedSequenceNeuralModel model;

    if (!deserializeCompositionConditionedSequenceNeuralModel(
            artifact,
            model))
    {
        fail("model artifact cannot be deserialized");
    }

    const auto knowledgeCatalog =
        buildKnowledgeCatalog(
            prepared.sequences,
            metadataLoad.catalog);

    if (!knowledgeCatalog.isValid())
        fail("knowledge catalog cannot be rebuilt");

    const auto partition =
        buildCompositionComposerKnowledgePartition(
            knowledgeCatalog,
            0.2,
            0.1);

    if (!partition.isValid(
            knowledgeCatalog.sampleCount()))
    {
        fail("partition is invalid");
    }

    if (partition.trainingCount() != 48 ||
        partition.validationCount() != 13 ||
        partition.testCount() != 6)
    {
        fail("reconstructed split does not match Phase 119");
    }

    const auto trainingDataset =
        buildSplitDataset(
            prepared.sequences,
            metadataLoad.catalog,
            knowledgeCatalog,
            partition.trainingSampleIndices);

    const auto validationDataset =
        buildSplitDataset(
            prepared.sequences,
            metadataLoad.catalog,
            knowledgeCatalog,
            partition.validationSampleIndices);

    const auto testDataset =
        buildSplitDataset(
            prepared.sequences,
            metadataLoad.catalog,
            knowledgeCatalog,
            partition.testSampleIndices);

    if (!trainingDataset.isValid() ||
        !validationDataset.isValid() ||
        !testDataset.isValid())
    {
        fail("one or more evaluation datasets are invalid");
    }

    if (!sameVector(
            model.vocabulary.composers,
            trainingDataset.vocabulary.composers) ||
        !sameVector(
            model.vocabulary.styles,
            trainingDataset.vocabulary.styles) ||
        !sameVector(
            model.vocabulary.eras,
            trainingDataset.vocabulary.eras) ||
        !sameVector(
            model.vocabulary.instrumentations,
            trainingDataset.vocabulary.instrumentations))
    {
        fail("model vocabulary is incompatible with corpus vocabulary");
    }

    const auto trainingEvaluation =
        evaluateCompositionConditionedSequenceNeuralModel(
            model,
            trainingDataset);

    const auto validationEvaluation =
        evaluateCompositionConditionedSequenceNeuralModel(
            model,
            validationDataset);

    const auto testEvaluation =
        evaluateCompositionConditionedSequenceNeuralModel(
            model,
            testDataset);

    printEvaluation(
        "train",
        trainingEvaluation);

    printEvaluation(
        "validation",
        validationEvaluation);

    printEvaluation(
        "test",
        testEvaluation);

    const auto trainingLoss =
        trainingEvaluation.loss;

    const auto validationGap =
        validationEvaluation.loss -
        trainingLoss;

    const auto testGap =
        testEvaluation.loss -
        trainingLoss;

    const auto validationRatio =
        validationGap /
        std::max(trainingLoss, 1.0e-12);

    const auto testRatio =
        testGap /
        std::max(trainingLoss, 1.0e-12);

    std::cout
        << "validationGap=" << validationGap << '\n'
        << "testGap=" << testGap << '\n'
        << "validationGapRatio=" << validationRatio << '\n'
        << "testGapRatio=" << testRatio << '\n'
        << "phase119ModelCompatible=1\n"
        << "PHASE 120.2 QUANTITATIVE EVALUATION COMPLETE\n";

    return 0;
}
