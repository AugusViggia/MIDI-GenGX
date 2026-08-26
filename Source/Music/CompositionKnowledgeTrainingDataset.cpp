#include "CompositionKnowledgeTrainingDataset.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <unordered_set>

namespace midigengx::music
{
namespace
{

bool isUnit(double value) noexcept
{
    return std::isfinite(value) &&
           value >= 0.0 &&
           value <= 1.0;
}

} // namespace

bool CompositionKnowledgeTrainingConditioning::isValid() const noexcept
{
    return valid &&
           !sampleId.empty() &&
           composerKnowledge.isValid() &&
           genreKnowledge.isValid() &&
           soundEngineeringKnowledge.isValid();
}

bool CompositionKnowledgeTrainingSample::isValid() const noexcept
{
    if (!valid ||
        sampleId.empty() ||
        sampleId != metadata.sampleId ||
        !metadata.isValid() ||
        !sequence.isValid() ||
        !composerKnowledge.isValid() ||
        !genreKnowledge.isValid() ||
        !soundEngineeringKnowledge.isValid())
    {
        return false;
    }

    if (composerKnowledge.sampleId != sampleId)
        return false;

    return std::all_of(
        conditioningFeatures.begin(),
        conditioningFeatures.end(),
        [](double value)
        {
            return isUnit(value);
        });
}

bool CompositionKnowledgeTrainingDataset::isValid() const noexcept
{
    if (!valid ||
        featureWidth !=
            CompositionKnowledgeTrainingSample::conditioningFeatureCount ||
        samples.empty())
    {
        return false;
    }

    std::unordered_set<std::string> ids;

    for (const auto& sample : samples)
    {
        if (!sample.isValid() ||
            !ids.insert(sample.sampleId).second)
        {
            return false;
        }
    }

    return verified && ids.size() == samples.size();
}

std::size_t CompositionKnowledgeTrainingDataset::sampleCount()
    const noexcept
{
    return samples.size();
}

CompositionKnowledgeTrainingDataset
buildCompositionKnowledgeTrainingDataset(
    const CompositionConditionedTrainingDataset& conditionedDataset,
    const std::vector<CompositionKnowledgeTrainingConditioning>& conditionings)
    noexcept
{
    CompositionKnowledgeTrainingDataset result;

    if (!conditionedDataset.isValid() ||
        conditionings.empty())
    {
        return result;
    }

    std::unordered_map<std::string, const CompositionKnowledgeTrainingConditioning*> conditioningBySample;
    conditioningBySample.reserve(conditionings.size());

    for (const auto& conditioning : conditionings)
    {
        if (!conditioning.isValid() ||
            !conditioningBySample.emplace(
                conditioning.sampleId,
                &conditioning).second)
        {
            return result;
        }
    }

    if (conditioningBySample.size() != conditionedDataset.samples.size())
        return result;

    result.samples.reserve(conditionedDataset.samples.size());

    for (const auto& conditionedSample : conditionedDataset.samples)
    {
        const auto it = conditioningBySample.find(
            conditionedSample.sequence.sampleId);

        if (it == conditioningBySample.end())
            return result;

        const auto& conditioning =
            *it->second;

        CompositionKnowledgeTrainingSample sample;
        sample.sampleId = conditionedSample.sequence.sampleId;
        sample.metadata = conditionedSample.metadata;
        sample.sequence = conditionedSample.sequence;
        sample.composerKnowledge = conditioning.composerKnowledge;
        sample.genreKnowledge = conditioning.genreKnowledge;
        sample.soundEngineeringKnowledge = conditioning.soundEngineeringKnowledge;

        std::size_t offset = 0;

        for (const auto value : sample.composerKnowledge.features)
            sample.conditioningFeatures[offset++] = value;

        for (const auto value : sample.genreKnowledge.features)
            sample.conditioningFeatures[offset++] = value;

        for (const auto value : sample.soundEngineeringKnowledge.features)
            sample.conditioningFeatures[offset++] = value;

        sample.valid = true;

        if (!sample.isValid())
            return CompositionKnowledgeTrainingDataset{};

        result.samples.push_back(std::move(sample));
    }

    std::sort(
        result.samples.begin(),
        result.samples.end(),
        [](const auto& left, const auto& right)
        {
            return left.sampleId < right.sampleId;
        });

    result.featureWidth =
        CompositionKnowledgeTrainingSample::conditioningFeatureCount;
    result.verified =
        conditionedDataset.verified &&
        std::all_of(
            result.samples.begin(),
            result.samples.end(),
            [](const auto& sample)
            {
                return sample.metadata.verified;
            });
    result.valid = true;

    if (!result.isValid())
        result.valid = false;

    return result;
}

} // namespace midigengx::music
