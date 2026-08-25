#include "Music/CompositionComposerKnowledgePartition.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

using namespace midigengx::music;

namespace
{

void expect(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}

CompositionComposerKnowledgeSample makeSample(
    const std::string& id,
    const std::string& composer,
    const std::string& work)
{
    CompositionDatasetSample composition;
    composition.sampleId = id;
    composition.globalFeatures = {0.25, 0.50, 0.25};
    composition.sectionFeatures =
    {
        {0.0, 0.2, 0.0, 0.25, 0.0, 0.0}
    };
    composition.analysisValid = true;

    CompositionSequenceMetadata metadata;
    metadata.sampleId = id;
    metadata.composerId = composer;
    metadata.workId = work;
    metadata.movementId = id;
    metadata.styleId = "style";
    metadata.eraId = "era";
    metadata.instrumentationId = "solo_piano";
    metadata.verified = true;
    metadata.valid = true;

    return buildCompositionComposerKnowledgeSample(
        composition,
        metadata);
}

CompositionComposerKnowledgeCatalog makeCatalog()
{
    return buildCompositionComposerKnowledgeCatalog(
    {
        makeSample("bach-w1-a", "bach", "w1"),
        makeSample("bach-w1-b", "bach", "w1"),
        makeSample("bach-w2", "bach", "w2"),
        makeSample("bach-w3", "bach", "w3"),
        makeSample("chopin-w1", "chopin", "w1"),
        makeSample("chopin-w2", "chopin", "w2"),
        makeSample("chopin-w3", "chopin", "w3"),
        makeSample("chopin-w4", "chopin", "w4")
    });
}

std::size_t indexOf(
    const CompositionComposerKnowledgeCatalog& catalog,
    const std::string& sampleId)
{
    std::size_t index = 0;

    for (const auto& composer :
         catalog.composers)
    {
        for (const auto& sample :
             composer.samples)
        {
            if (sample.composition.sampleId ==
                sampleId)
            {
                return index;
            }

            ++index;
        }
    }

    return index;
}

ComposerKnowledgeSplit splitFor(
    const CompositionComposerKnowledgePartition& partition,
    std::size_t index)
{
    if (partition.contains(
            ComposerKnowledgeSplit::Test,
            index))
    {
        return ComposerKnowledgeSplit::Test;
    }

    if (partition.contains(
            ComposerKnowledgeSplit::Validation,
            index))
    {
        return ComposerKnowledgeSplit::Validation;
    }

    return ComposerKnowledgeSplit::Training;
}

void testPartitionIsValid()
{
    const auto catalog = makeCatalog();

    const auto partition =
        buildCompositionComposerKnowledgePartition(
            catalog,
            0.25,
            0.25);

    expect(
        partition.isValid(
            catalog.sampleCount()),
        "composer knowledge partition is valid");

    expect(
        partition.trainingCount() +
            partition.validationCount() +
            partition.testCount() ==
            catalog.sampleCount(),
        "all samples are covered exactly once");
}

void testSplitDoesNotCrossWorkBoundary()
{
    const auto catalog = makeCatalog();

    const auto partition =
        buildCompositionComposerKnowledgePartition(
            catalog,
            0.25,
            0.25);

    for (const auto& composer :
         catalog.composers)
    {
        std::vector<std::string> seenWorks;

        for (const auto& sample :
             composer.samples)
        {
            bool known = false;
            ComposerKnowledgeSplit expected =
                ComposerKnowledgeSplit::Training;

            for (std::size_t i = 0;
                 i < composer.samples.size();
                 ++i)
            {
                if (composer.samples[i].metadata.workId ==
                    sample.metadata.workId &&
                    i < &sample -
                        &composer.samples.front())
                {
                    known = true;

                    const auto globalIndex =
                        indexOf(
                            catalog,
                            composer.samples[i]
                                .composition.sampleId);

                    expected =
                        splitFor(
                            partition,
                            globalIndex);

                    break;
                }
            }

            if (known)
            {
                const auto globalIndex =
                    indexOf(
                        catalog,
                        sample.composition.sampleId);

                expect(
                    splitFor(
                        partition,
                        globalIndex) ==
                        expected,
                    "all samples of one work stay in one split");
            }

            seenWorks.push_back(
                sample.metadata.workId);
        }
    }
}

void testComposerTrainingCoverage()
{
    const auto catalog = makeCatalog();

    const auto partition =
        buildCompositionComposerKnowledgePartition(
            catalog,
            0.25,
            0.25);

    for (const auto& composer :
         catalog.composers)
    {
        bool hasTraining =
            false;

        for (const auto& sample :
             composer.samples)
        {
            const auto globalIndex =
                indexOf(
                    catalog,
                    sample.composition.sampleId);

            if (partition.contains(
                    ComposerKnowledgeSplit::Training,
                    globalIndex))
            {
                hasTraining = true;
                break;
            }
        }

        expect(
            hasTraining,
            "every composer has training data");
    }
}

void testPartitionIsDeterministic()
{
    const auto catalog = makeCatalog();

    const auto first =
        buildCompositionComposerKnowledgePartition(
            catalog,
            0.25,
            0.25);

    const auto second =
        buildCompositionComposerKnowledgePartition(
            catalog,
            0.25,
            0.25);

    expect(
        first.trainingSampleIndices ==
            second.trainingSampleIndices &&
        first.validationSampleIndices ==
            second.validationSampleIndices &&
        first.testSampleIndices ==
            second.testSampleIndices,
        "composer knowledge partition is deterministic");
}

} // namespace

int main()
{
    testPartitionIsValid();
    testSplitDoesNotCrossWorkBoundary();
    testComposerTrainingCoverage();
    testPartitionIsDeterministic();

    std::cout
        << "MIDI-GenGX Phase 98 composer knowledge partition tests passed.\n";
    return 0;
}
