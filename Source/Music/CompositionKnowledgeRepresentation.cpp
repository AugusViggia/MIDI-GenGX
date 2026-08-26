#include "CompositionKnowledgeRepresentation.h"

#include "CompositionDatasetSchema.h"

#include <cmath>

namespace midigengx::music
{

bool CompositionKnowledgeRepresentation::isValid() const noexcept
{
    if (!analysisValid ||
        !snapshot.isValid() ||
        !datasetSample.isValid() ||
        datasetSample.schemaVersion !=
            CompositionDatasetSchema::version ||
        datasetSample.sampleId.empty() ||
        datasetSample.sectionCount() !=
            snapshot.sectionCount() ||
        datasetSample.globalFeatures.size() !=
            CompositionDatasetSchema::globalFeatureCount)
    {
        return false;
    }

    for (const auto value : datasetSample.globalFeatures)
    {
        if (!std::isfinite(value) ||
            value < -1.0 ||
            value > 1.0)
        {
            return false;
        }
    }

    for (const auto& section : datasetSample.sectionFeatures)
    {
        if (section.size() !=
            CompositionDatasetSchema::sectionFeatureCount)
        {
            return false;
        }

        for (const auto value : section)
        {
            if (!std::isfinite(value) ||
                value < -1.0 ||
                value > 1.0)
            {
                return false;
            }
        }
    }

    return true;
}

std::size_t CompositionKnowledgeRepresentation::sectionCount() const noexcept
{
    return snapshot.sectionCount();
}

std::size_t CompositionKnowledgeRepresentation::globalFeatureWidth() const noexcept
{
    return datasetSample.globalFeatures.size();
}

std::size_t CompositionKnowledgeRepresentation::sectionFeatureWidth() const noexcept
{
    if (datasetSample.sectionFeatures.empty())
        return 0;

    return datasetSample.sectionFeatures.front().size();
}

CompositionKnowledgeRepresentation
buildCompositionKnowledgeRepresentation(
    const CompositionKnowledgeSnapshot& sourceSnapshot,
    const std::string& sampleId) noexcept
{
    CompositionKnowledgeRepresentation result;

    if (!sourceSnapshot.isValid() ||
        sampleId.empty())
    {
        return result;
    }

    const auto sample =
        buildCompositionDatasetSample(
            sourceSnapshot,
            sampleId);

    if (!sample.isValid() ||
        sample.schemaVersion !=
            CompositionDatasetSchema::version)
    {
        return result;
    }

    result.snapshot = sourceSnapshot;
    result.datasetSample = sample;
    result.analysisValid = true;

    if (!result.isValid())
    {
        result = {};
    }

    return result;
}

} // namespace midigengx::music
