#include "Music/CompositionConditionedSequenceNeuralModelRuntimeLoader.h"
#include "Music/CompositionConditionedSequenceNeuralModelArtifact.h"
#include "Music/CompositionConditionedSequenceNeuralEvaluator.h"
#include "Music/CompositionRealComposerCorpusPreparation.h"
#include "Music/CompositionComposerKnowledgeSample.h"
#include "Music/CompositionComposerKnowledgeCatalog.h"
#include "Music/CompositionComposerKnowledgePartition.h"
#include "Music/CompositionSequenceMetadataFileLoader.h"

#include <cstdint>
#include <limits>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

using namespace midigengx::music;

namespace
{
std::vector<std::uint8_t> readFile(const std::string& path)
{
    std::ifstream input(path, std::ios::binary);
    if (!input)
        return {};

    return std::vector<std::uint8_t>(
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>());
}

bool inspect(
    const std::string& label,
    const std::string& path,
    const std::string& corpus,
    const std::string& metadataPath,
    std::ostream& out)
{
    const auto bytes = readFile(path);

    if (bytes.empty())
    {
        out
            << label
            << ".fileValid=0\n";
        return false;
    }

    const auto loaded =
        CompositionConditionedSequenceNeuralModelRuntimeLoader{}
            .load(bytes);

    if (!loaded.isValid())
    {
        out
            << label
            << ".modelLoaded=0\n";
        return false;
    }

    const auto metadata =
        loadCompositionSequenceMetadataFile(
            metadataPath);

    if (!metadata.isValid())
    {
        out
            << label
            << ".metadataValid=0\n";
        return false;
    }

    const auto prepared =
        prepareRealComposerCorpusFromDirectory(
            corpus,
            true,
            metadata.catalog);

    if (!prepared.isValid())
    {
        out
            << label
            << ".corpusPrepared=0\n";
        return false;
    }

    if (prepared.sequences.empty())
    {
        out
            << label
            << ".sequenceCount=0\n";
        return false;
    }

    // Rebuild the established training dataset exactly as the training path
    // does so that baseline and final use the same 67-sample contract.
    std::vector<CompositionComposerKnowledgeSample>
        knowledgeSamples;

    knowledgeSamples.reserve(
        prepared.sequences.size());

    for (const auto& sequence :
         prepared.sequences)
    {
        const auto* sampleMetadata =
            metadata.catalog.findBySampleId(
                sequence.sampleId);

        if (sampleMetadata == nullptr)
            return false;

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
                *sampleMetadata);

        if (!knowledge.isValid())
            return false;

        knowledgeSamples.push_back(
            knowledge);
    }

    const auto knowledgeCatalog =
        buildCompositionComposerKnowledgeCatalog(
            knowledgeSamples);

    if (!knowledgeCatalog.isValid())
        return false;

    const auto partition =
        buildCompositionComposerKnowledgePartition(
            knowledgeCatalog,
            0.2,
            0.1);

    if (!partition.isValid(
            knowledgeCatalog.sampleCount()))
    {
        return false;
    }

    if (partition.trainingCount() != 48 ||
        partition.validationCount() != 13 ||
        partition.testCount() != 6)
    {
        return false;
    }

    std::vector<CompositionComposerKnowledgeSample>
        flattened;

    for (const auto& composer :
         knowledgeCatalog.composers)
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
            return false;

        const auto& target =
            flattened[index];

        for (const auto& sequence :
             prepared.sequences)
        {
            if (sequence.sampleId ==
                target.composition.sampleId)
            {
                trainingSequences.push_back(
                    sequence);
                break;
            }
        }
    }

    const auto dataset =
        buildCompositionConditionedTrainingDataset(
            trainingSequences,
            metadata.catalog);

    if (!dataset.isValid() ||
        dataset.sampleCount() != 48)
    {
        return false;
    }

    if (loaded.model.vocabulary.composers !=
            dataset.vocabulary.composers ||
        loaded.model.vocabulary.styles !=
            dataset.vocabulary.styles ||
        loaded.model.vocabulary.eras !=
            dataset.vocabulary.eras ||
        loaded.model.vocabulary.instrumentations !=
            dataset.vocabulary.instrumentations)
    {
        return false;
    }

    const auto contract =
        loaded.model.contract;

    auto evaluateLoss =
        [&]()
        {
            double total = 0.0;
            std::size_t windows = 0;

            for (const auto& sample :
                 dataset.samples)
            {
                const auto windowsForSample =
                    buildCompositionMidiSequenceWindows(
                        sample.sequence,
                        contract);

                for (const auto& window :
                     windowsForSample)
                {
                    const auto prediction =
                        loaded.model.predictNextEvent(
                            window,
                            sample);

                    if (!prediction.isValid(
                            contract.targetFeatureWidth))
                    {
                        return std::numeric_limits<double>::infinity();
                    }

                    for (std::size_t index = 0;
                         index < prediction.features.size();
                         ++index)
                    {
                        const auto error =
                            prediction.features[index] -
                            window.targets[index];

                        total +=
                            error * error;
                    }

                    ++windows;
                }
            }

            if (windows == 0)
                return std::numeric_limits<double>::infinity();

            return total /
                   static_cast<double>(
                       windows *
                       contract.targetFeatureWidth);
        };

    const auto loss =
        evaluateLoss();

    if (!std::isfinite(loss))
    {
        out
            << label
            << ".evaluationValid=0\n";
        return false;
    }

    out
        << label
        << ".fileValid=1\n"
        << label
        << ".modelLoaded=1\n"
        << label
        << ".sampleCount="
        << prepared.sequences.size()
        << '\n'
        << label
        << ".averageLoss="
        << std::setprecision(12)
        << loss
        << '\n';

    return true;
}


} // namespace

int main(int argc, char* argv[])
{
    if (argc != 5)
    {
        std::cerr
            << "FAILED: usage: Phase121BaselineVsFinalEvaluation "
            << "<corpus> <metadata.tsv> <phase119.mgcn> <phase121.mgcn>\n";
        return 1;
    }

    const std::string corpus = argv[1];
    const std::string metadata = argv[2];
    const std::string baseline = argv[3];
    const std::string finalModel = argv[4];

    std::ofstream report(
        "phase121-baseline-vs-final-evaluation.txt");

    if (!report)
    {
        std::cerr
            << "FAILED: cannot create project-root report\n";
        return 1;
    }

    report
        << "PHASE 121 BASELINE VS FINAL EVALUATION\n";

    const bool baselineOk =
        inspect(
            "phase119",
            baseline,
            corpus,
            metadata,
            report);

    const bool finalOk =
        inspect(
            "phase121",
            finalModel,
            corpus,
            metadata,
            report);

    if (!baselineOk || !finalOk)
        return 1;

    report
        << "evaluationComplete=1\n";

    std::ifstream verify(
        "phase121-baseline-vs-final-evaluation.txt");

    std::cout
        << verify.rdbuf();

    return 0;
}
