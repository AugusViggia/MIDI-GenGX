#include "CompositionComposerKnowledgeCorpusAssembly.h"

#include <unordered_map>
#include <unordered_set>

namespace midigengx::music
{

bool CompositionComposerKnowledgeCorpusAssembly::isValid()
    const noexcept
{
    return valid &&
           sourceIntegrityVerified &&
           sourceManifest.isValid() &&
           trainingCorpus.isValid() &&
           sourceManifest.sampleCount() ==
               trainingCorpus.sampleCount();
}

std::size_t
CompositionComposerKnowledgeCorpusAssembly::sampleCount()
    const noexcept
{
    return trainingCorpus.sampleCount();
}

CompositionComposerKnowledgeCorpusAssembly
assembleCompositionComposerKnowledgeCorpus(
    const CompositionComposerCorpusSourceManifest& sourceManifest,
    const CompositionComposerKnowledgeTrainingCorpus& trainingCorpus)
    noexcept
{
    CompositionComposerKnowledgeCorpusAssembly result;

    if (!sourceManifest.isValid() ||
        !trainingCorpus.isValid() ||
        sourceManifest.sampleCount() !=
            trainingCorpus.sampleCount())
    {
        return result;
    }

    std::unordered_map<
        std::string,
        const CompositionComposerCorpusSourceEntry*>
        sourceById;

    for (const auto& entry :
         sourceManifest.entries)
    {
        if (!sourceById.emplace(
                entry.sampleId,
                &entry).second)
        {
            return result;
        }
    }

    std::unordered_set<std::string> trainingIds;

    const auto checkSamples =
        [&](const auto& samples)
        {
            for (const auto& sample :
                 samples)
            {
                const auto& sampleId =
                    sample.composition.sampleId;

                if (sample.metadata.sampleId !=
                        sampleId ||
                    !trainingIds.insert(
                        sampleId).second)
                {
                    return false;
                }

                if (sourceById.find(
                        sampleId) ==
                    sourceById.end())
                {
                    return false;
                }
            }

            return true;
        };

    if (!checkSamples(
            trainingCorpus.trainingSamples) ||
        !checkSamples(
            trainingCorpus.validationSamples) ||
        !checkSamples(
            trainingCorpus.testSamples))
    {
        return {};
    }

    result.sourceManifest =
        sourceManifest;

    result.trainingCorpus =
        trainingCorpus;

    result.sourceIntegrityVerified =
        true;

    result.valid =
        true;

    if (!result.isValid())
        result.valid = false;

    return result;
}

} // namespace midigengx::music
