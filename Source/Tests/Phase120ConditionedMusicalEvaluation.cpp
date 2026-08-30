#include "Music/CompositionConditionedSequenceNeuralEvaluator.h"
#include "Music/CompositionConditionedSequenceNeuralModelArtifact.h"
#include "Music/CompositionComposerKnowledgeCatalog.h"
#include "Music/CompositionComposerKnowledgePartition.h"
#include "Music/CompositionConditionedTrainingDataset.h"
#include "Music/CompositionRealComposerCorpusPreparation.h"
#include "Music/CompositionSequenceMetadataFileLoader.h"
#include "Music/CompositionComposerKnowledgeSample.h"
#include "Music/CompositionMidiSequenceWindow.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

using namespace midigengx::music;

namespace
{

struct MusicalEvaluationSummary
{
    std::size_t sampleCount = 0;
    std::size_t windowCount = 0;

    double melodicScore = 0.0;
    double rhythmicScore = 0.0;
    double harmonicScore = 0.0;
    double structuralScore = 0.0;
    double rangeValidityScore = 0.0;
    double overallScore = 0.0;

    bool valid = false;
};

void fail(const std::string& message)
{
    std::cerr << "FAILED: " << message << '\n';
    std::exit(1);
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

double closeness(
    double predicted,
    double target) noexcept
{
    return std::clamp(
        1.0 - std::abs(predicted - target) / 2.0,
        0.0,
        1.0);
}

double groupCloseness(
    const std::vector<double>& predicted,
    const std::vector<double>& target,
    std::initializer_list<std::size_t> indices) noexcept
{
    if (indices.size() == 0)
        return 0.0;

    double total = 0.0;

    for (const auto index : indices)
    {
        if (index >= predicted.size() ||
            index >= target.size())
        {
            return 0.0;
        }

        total +=
            closeness(
                predicted[index],
                target[index]);
    }

    return total /
           static_cast<double>(
               indices.size());
}

MusicalEvaluationSummary
evaluateDataset(
    const CompositionConditionedSequenceNeuralModel& model,
    const CompositionConditionedTrainingDataset& dataset)
{
    MusicalEvaluationSummary result;

    if (!model.isValid() ||
        !dataset.isValid())
        return result;

    double melodic = 0.0;
    double rhythmic = 0.0;
    double harmonic = 0.0;
    double structural = 0.0;
    double rangeValidity = 0.0;

    std::size_t windows = 0;

    const auto contract =
        model.contract;

    for (const auto& sample :
         dataset.samples)
    {
        const auto sequenceWindows =
            buildCompositionMidiSequenceWindows(
                sample.sequence,
                contract);

        for (const auto& window :
             sequenceWindows)
        {
            const auto prediction =
                model.predictNextEvent(
                    window,
                    sample);

            if (!prediction.isValid(
                    contract.targetFeatureWidth))
            {
                return MusicalEvaluationSummary{};
            }

            melodic +=
                groupCloseness(
                    prediction.features,
                    window.targets,
                    {0, 6, 7, 8});

            rhythmic +=
                groupCloseness(
                    prediction.features,
                    window.targets,
                    {1, 2, 3, 4, 5, 9, 19});

            harmonic +=
                groupCloseness(
                    prediction.features,
                    window.targets,
                    {14, 15, 16, 17});

            structural +=
                groupCloseness(
                    prediction.features,
                    window.targets,
                    {10, 11, 12, 13, 18});

            std::size_t validValues = 0;

            for (const auto value :
                 prediction.features)
            {
                if (std::isfinite(value) &&
                    value >= -1.0 &&
                    value <= 1.0)
                {
                    ++validValues;
                }
            }

            rangeValidity +=
                static_cast<double>(validValues) /
                static_cast<double>(prediction.features.size());

            ++windows;
        }
    }

    if (windows == 0)
        return result;

    result.sampleCount = dataset.samples.size();
    result.windowCount = windows;

    result.melodicScore =
        melodic / static_cast<double>(windows);
    result.rhythmicScore =
        rhythmic / static_cast<double>(windows);
    result.harmonicScore =
        harmonic / static_cast<double>(windows);
    result.structuralScore =
        structural / static_cast<double>(windows);
    result.rangeValidityScore =
        rangeValidity / static_cast<double>(windows);

    result.overallScore =
        0.30 * result.melodicScore +
        0.25 * result.rhythmicScore +
        0.20 * result.harmonicScore +
        0.15 * result.structuralScore +
        0.10 * result.rangeValidityScore;

    result.valid =
        std::isfinite(result.overallScore) &&
        result.overallScore >= 0.0 &&
        result.overallScore <= 1.0;

    return result;
}

void printSummary(
    const char* name,
    const MusicalEvaluationSummary& result)
{
    if (!result.valid)
        fail(std::string(name) + " musical evaluation is invalid");

    std::cout
        << name << ".sampleCount=" << result.sampleCount << '\n'
        << name << ".windowCount=" << result.windowCount << '\n'
        << name << ".melodicScore=" << result.melodicScore << '\n'
        << name << ".rhythmicScore=" << result.rhythmicScore << '\n'
        << name << ".harmonicScore=" << result.harmonicScore << '\n'
        << name << ".structuralScore=" << result.structuralScore << '\n'
        << name << ".rangeValidityScore=" << result.rangeValidityScore << '\n'
        << name << ".overallScore=" << result.overallScore << '\n';
}

} // namespace

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        fail(
            "usage: Phase120ConditionedMusicalEvaluation "
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

    if (!validationDataset.isValid() ||
        !testDataset.isValid())
    {
        fail("one or more evaluation datasets are invalid");
    }

    const auto validationSummary =
        evaluateDataset(
            model,
            validationDataset);

    const auto testSummary =
        evaluateDataset(
            model,
            testDataset);

    printSummary(
        "validation",
        validationSummary);

    printSummary(
        "test",
        testSummary);

    const auto musicalGeneralizationGap =
        validationSummary.overallScore -
        testSummary.overallScore;

    std::cout
        << "musicalGeneralizationGap="
        << musicalGeneralizationGap << '\n'
        << "phase119ModelCompatible=1\n"
        << "PHASE 120.3 CONDITIONED REPRESENTATION MUSICAL EVALUATION COMPLETE\n";

    return 0;
}
