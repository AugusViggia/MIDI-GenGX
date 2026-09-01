#include "CompositionComposerKnowledgeTrainingCorpus.h"

#include <algorithm>
#include <vector>

namespace midigengx::music
{

bool CompositionComposerKnowledgeTrainingCorpus::isValid() const noexcept
{
    if (!valid || !manifest.isValid())
        return false;

    if (trainingCount() != manifest.trainingCount ||
        validationCount() != manifest.validationCount ||
        testCount() != manifest.testCount ||
        sampleCount() != manifest.sampleCount)
        return false;

    const auto validSamples =
        [](const auto& samples)
        {
            return std::all_of(
                samples.begin(),
                samples.end(),
                [](const auto& sample)
                {
                    return sample.isValid();
                });
        };

    return validSamples(trainingSamples) &&
           validSamples(validationSamples) &&
           validSamples(testSamples);
}

std::size_t CompositionComposerKnowledgeTrainingCorpus::trainingCount() const noexcept
{
    return trainingSamples.size();
}

std::size_t CompositionComposerKnowledgeTrainingCorpus::validationCount() const noexcept
{
    return validationSamples.size();
}

std::size_t CompositionComposerKnowledgeTrainingCorpus::testCount() const noexcept
{
    return testSamples.size();
}

std::size_t CompositionComposerKnowledgeTrainingCorpus::sampleCount() const noexcept
{
    return trainingCount() +
           validationCount() +
           testCount();
}

CompositionComposerKnowledgeTrainingCorpus
buildCompositionComposerKnowledgeTrainingCorpus(
    const CompositionComposerKnowledgeCatalog& catalog,
    const CompositionComposerKnowledgePartition& partition,
    const CompositionComposerKnowledgeCorpusManifest& manifest) noexcept
{
    CompositionComposerKnowledgeTrainingCorpus corpus;

    if (!catalog.isValid() ||
        !partition.isValid(catalog.sampleCount()) ||
        !manifest.isValid() ||
        manifest.sampleCount != catalog.sampleCount() ||
        manifest.trainingCount != partition.trainingCount() ||
        manifest.validationCount != partition.validationCount() ||
        manifest.testCount != partition.testCount())
    {
        return corpus;
    }

    // Partition indices address the deterministic global catalog order. Keep
    // references to the existing samples instead of copying the entire catalog
    // into a temporary vector before selecting each split.
    std::vector<const CompositionComposerKnowledgeSample*> flattened;
    flattened.reserve(catalog.sampleCount());

    for (const auto& composer : catalog.composers)
    {
        for (const auto& sample : composer.samples)
            flattened.push_back(&sample);
    }

    const auto append =
        [&flattened](
            std::vector<CompositionComposerKnowledgeSample>& target,
            const std::vector<std::size_t>& indices)
        {
            target.reserve(indices.size());

            for (const auto index : indices)
            {
                if (index >= flattened.size() ||
                    flattened[index] == nullptr)
                {
                    return false;
                }

                target.push_back(*flattened[index]);
            }

            return true;
        };

    if (!append(corpus.trainingSamples, partition.trainingSampleIndices) ||
        !append(corpus.validationSamples, partition.validationSampleIndices) ||
        !append(corpus.testSamples, partition.testSampleIndices))
    {
        return {};
    }

    corpus.manifest = manifest;
    corpus.valid = true;

    if (!corpus.isValid())
        corpus.valid = false;

    return corpus;
}

} // namespace midigengx::music
