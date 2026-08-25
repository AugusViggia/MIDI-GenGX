#include "CompositionComposerKnowledgeCorpusManifest.h"

#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace midigengx::music
{

bool CompositionComposerKnowledgeCorpusManifest::isValid()
    const noexcept
{
    if (!valid ||
        !verified ||
        corpusId.empty() ||
        corpusVersion.empty() ||
        composerCount == 0 ||
        sampleCount == 0 ||
        composerIds.size() != composerCount ||
        trainingCount +
                validationCount +
                testCount !=
            sampleCount ||
        validationRatio < 0.0 ||
        testRatio < 0.0 ||
        validationRatio >= 1.0 ||
        testRatio >= 1.0 ||
        validationRatio +
                testRatio >=
            1.0)
    {
        return false;
    }

    if (!std::is_sorted(
            composerIds.begin(),
            composerIds.end()))
    {
        return false;
    }

    if (std::adjacent_find(
            composerIds.begin(),
            composerIds.end()) !=
        composerIds.end())
    {
        return false;
    }

    return true;
}

CompositionComposerKnowledgeCorpusManifest
buildCompositionComposerKnowledgeCorpusManifest(
    const CompositionComposerKnowledgeCatalog& catalog,
    const CompositionComposerKnowledgePartition& partition,
    const std::string& corpusId,
    const std::string& corpusVersion)
    noexcept
{
    CompositionComposerKnowledgeCorpusManifest manifest;

    if (!catalog.isValid() ||
        !partition.isValid(
            catalog.sampleCount()) ||
        corpusId.empty() ||
        corpusVersion.empty())
    {
        return manifest;
    }

    std::unordered_set<std::string> works;

    for (const auto& composer :
         catalog.composers)
    {
        for (const auto& sample :
             composer.samples)
        {
            works.insert(
                composer.composerId +
                "\x1f" +
                sample.metadata.workId);
        }
    }

    manifest.corpusId =
        corpusId;

    manifest.corpusVersion =
        corpusVersion;

    manifest.composerCount =
        catalog.composerCount();

    manifest.sampleCount =
        catalog.sampleCount();

    manifest.workCount =
        works.size();

    manifest.validationRatio =
        partition.validationRatio;

    manifest.testRatio =
        partition.testRatio;

    manifest.trainingCount =
        partition.trainingCount();

    manifest.validationCount =
        partition.validationCount();

    manifest.testCount =
        partition.testCount();

    for (const auto& composer :
         catalog.composers)
    {
        manifest.composerIds.push_back(
            composer.composerId);
    }

    manifest.verified =
        catalog.verified;

    manifest.valid =
        true;

    if (!manifest.isValid())
        manifest.valid = false;

    return manifest;
}

} // namespace midigengx::music
