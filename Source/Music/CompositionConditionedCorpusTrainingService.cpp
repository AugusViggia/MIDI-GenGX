#include "CompositionConditionedCorpusTrainingService.h"

#include "CompositionRealComposerCorpusPreparation.h"
#include "CompositionComposerKnowledgeSample.h"
#include "CompositionComposerKnowledgeCatalog.h"
#include "CompositionMidiCorpusDirectoryLoader.h"
#include "CompositionMidiTrainingCorpusArtifact.h"
#include "CompositionConditionedSequenceNeuralModelArtifact.h"

#include <unordered_set>
#include <vector>

namespace midigengx::music
{

bool CompositionConditionedCorpusTrainingResult::isValid()
    const noexcept
{
    return valid &&
           inputFileCount > 0 &&
           sequenceCount > 0 &&
           sequenceCount +
               rejectedFileCount ==
               inputFileCount &&
           sequenceCorpusArtifact.isValid() &&
           metadataArtifact.isValid() &&
           trainingRun.isValid() &&
           trainingSampleCount > 0 &&
           trainingSampleCount +
               validationSampleCount +
               testSampleCount ==
           sequenceCount &&
           ((!validationEvaluation.valid && validationSampleCount == 0) ||
            (validationEvaluation.valid &&
             validationEvaluation.sampleCount ==
                 validationSampleCount)) &&
           ((!testEvaluation.valid && testSampleCount == 0) ||
            (testEvaluation.valid &&
             testEvaluation.sampleCount ==
                 testSampleCount));
}

CompositionConditionedCorpusTrainingResult
runCompositionConditionedCorpusTraining(
    const std::string& midiDirectoryPath,
    bool recursive,
    const CompositionSequenceMetadataCatalog& metadataCatalog,
    const CompositionConditionedSequenceNeuralTrainingConfig& config)
    noexcept
{
    CompositionConditionedCorpusTrainingResult result;

    if (!metadataCatalog.isValid() ||
        !metadataCatalog.verified ||
        midiDirectoryPath.empty())
    {
        return result;
    }

    const auto loaded =
        loadCompositionMidiCorpusDirectory(
            midiDirectoryPath,
            recursive);

    if (!loaded.isValid() ||
        loaded.records.empty())
    {
        return result;
    }

    // Production composer learning must have a complete metadata contract:
    // every accepted MIDI sample needs a verified metadata entry, and every
    // metadata entry must correspond to a discovered MIDI sample.
    std::unordered_set<std::string> midiSampleIds;

    for (const auto& record :
         loaded.records)
    {
        if (!midiSampleIds.insert(
                record.sampleId).second)
        {
            return result;
        }
    }

    std::unordered_set<std::string> metadataSampleIds;

    for (const auto& metadata :
         metadataCatalog.entries)
    {
        if (!metadata.verified ||
            !metadata.isValid() ||
            !metadataSampleIds.insert(
                metadata.sampleId).second ||
            midiSampleIds.find(
                metadata.sampleId) ==
                midiSampleIds.end())
        {
            return result;
        }
    }

    if (metadataSampleIds.size() !=
            midiSampleIds.size())
    {
        return result;
    }

    for (const auto& sampleId :
         midiSampleIds)
    {
        if (metadataSampleIds.find(
                sampleId) ==
            metadataSampleIds.end())
        {
            return result;
        }
    }

    const auto prepared =
        prepareRealComposerCorpusFromRecords(
            loaded.records,
            metadataCatalog);

    if (!prepared.isValid())
        return result;

    std::vector<CompositionMidiTrainingSequence>
        sequences =
            prepared.sequences;

    result.inputFileCount =
        loaded.discoveredFileCount;

    result.sequenceCount =
        sequences.size();

    result.rejectedFileCount =
        loaded.rejectedFileCount +
        prepared.rejectedSampleCount;

    if (result.sequenceCount +
            result.rejectedFileCount !=
        result.inputFileCount)
    {
        return result;
    }

    result.sequenceCorpusArtifact =
        serializeCompositionMidiTrainingSequences(
            sequences);

    if (!result.sequenceCorpusArtifact.isValid())
        return result;

    result.metadataArtifact =
        serializeCompositionSequenceMetadataCatalog(
            metadataCatalog);

    if (!result.metadataArtifact.isValid())
        return result;

    // Build the established composer-aware catalog from the prepared real
    // sequences. This is the first point where the train/validation/test
    // boundary becomes authoritative for the neural training run.
    std::vector<CompositionComposerKnowledgeSample>
        knowledgeSamples;

    knowledgeSamples.reserve(
        sequences.size());

    for (const auto& sequence :
         sequences)
    {
        const auto* metadata =
            metadataCatalog.findBySampleId(
                sequence.sampleId);

        if (metadata == nullptr)
            return result;

        CompositionDatasetSample composition;
        composition.sampleId =
            sequence.sampleId;
        composition.globalFeatures =
            {
                0.0,
                0.0,
                0.0
            };
        composition.analysisValid =
            true;

        const auto knowledge =
            buildCompositionComposerKnowledgeSample(
                composition,
                *metadata);

        if (!knowledge.isValid())
            return result;

        knowledgeSamples.push_back(
            knowledge);
    }

    const auto knowledgeCatalog =
        buildCompositionComposerKnowledgeCatalog(
            knowledgeSamples);

    if (!knowledgeCatalog.isValid())
        return result;

    const auto partition =
        buildCompositionComposerKnowledgePartition(
            knowledgeCatalog,
            0.2,
            0.1);

    if (!partition.isValid(
            knowledgeCatalog.sampleCount()))
    {
        return result;
    }

    const auto corpusManifest =
        buildCompositionComposerKnowledgeCorpusManifest(
            knowledgeCatalog,
            partition,
            "real-composer-training",
            "1");

    if (!corpusManifest.isValid())
        return result;

    const auto trainingCorpus =
        buildCompositionComposerKnowledgeTrainingCorpus(
            knowledgeCatalog,
            partition,
            corpusManifest);

    if (!trainingCorpus.isValid() ||
        trainingCorpus.trainingSamples.empty())
    {
        return result;
    }

    result.trainingSampleCount =
        trainingCorpus.trainingCount();

    result.validationSampleCount =
        trainingCorpus.validationCount();

    result.testSampleCount =
        trainingCorpus.testCount();

    std::vector<CompositionMidiTrainingSequence>
        trainingSequences;

    trainingSequences.reserve(
        trainingCorpus.trainingSamples.size());

    for (const auto& sample :
         trainingCorpus.trainingSamples)
    {
        trainingSequences.push_back(
            sample.composition.sampleId.empty()
                ? CompositionMidiTrainingSequence{}
                : [&]()
                {
                    for (const auto& candidate :
                         sequences)
                    {
                        if (candidate.sampleId ==
                            sample.composition.sampleId)
                        {
                            return candidate;
                        }
                    }

                    return CompositionMidiTrainingSequence{};
                }());
    }

    for (const auto& trainingSequence :
         trainingSequences)
    {
        if (!trainingSequence.isValid())
            return result;
    }

    const auto trainingDataset =
        buildCompositionConditionedTrainingDataset(
            trainingSequences,
            metadataCatalog);

    if (!trainingDataset.isValid())
        return result;

    result.trainingRun =
        runCompositionConditionedTraining(
            serializeCompositionMidiTrainingSequences(
                trainingSequences),
            result.metadataArtifact,
            config);

    if (!result.trainingRun.isValid())
        return result;

    CompositionConditionedSequenceNeuralModel trainedModel;

    if (!deserializeCompositionConditionedSequenceNeuralModel(
            result.trainingRun.modelArtifact,
            trainedModel))
    {
        return result;
    }

    const auto buildSplitSequences =
        [&sequences](
            const std::vector<
                CompositionComposerKnowledgeSample>& splitSamples)
        {
            std::vector<CompositionMidiTrainingSequence>
                splitSequences;

            splitSequences.reserve(
                splitSamples.size());

            for (const auto& sample :
                 splitSamples)
            {
                for (const auto& candidate :
                     sequences)
                {
                    if (candidate.sampleId ==
                        sample.composition.sampleId)
                    {
                        splitSequences.push_back(
                            candidate);
                        break;
                    }
                }
            }

            return splitSequences;
        };

    const auto validationSequences =
        buildSplitSequences(
            trainingCorpus.validationSamples);

    if (!validationSequences.empty())
    {
        const auto validationDataset =
            buildCompositionConditionedTrainingDataset(
                validationSequences,
                metadataCatalog);

        if (!validationDataset.isValid())
            return result;

        result.validationEvaluation =
            evaluateCompositionConditionedSequenceNeuralModel(
                trainedModel,
                validationDataset);

        if (!result.validationEvaluation.isValid())
            return result;
    }

    const auto testSequences =
        buildSplitSequences(
            trainingCorpus.testSamples);

    if (!testSequences.empty())
    {
        const auto testDataset =
            buildCompositionConditionedTrainingDataset(
                testSequences,
                metadataCatalog);

        if (!testDataset.isValid())
            return result;

        result.testEvaluation =
            evaluateCompositionConditionedSequenceNeuralModel(
                trainedModel,
                testDataset);

        if (!result.testEvaluation.isValid())
            return result;
    }

    result.valid =
        true;

    if (!result.isValid())
        result.valid = false;

    return result;
}

} // namespace midigengx::music
