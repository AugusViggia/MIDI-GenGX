#include "CompositionComposerKnowledgePartition.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace midigengx::music
{
namespace
{

std::uint64_t stableHash(
    const std::string& value) noexcept
{
    std::uint64_t hash =
        14695981039346656037ull;

    for (const unsigned char character :
         value)
    {
        hash ^=
            static_cast<std::uint64_t>(
                character);

        hash *=
            1099511628211ull;
    }

    return hash;
}

struct RankedWork
{
    std::uint64_t hash = 0;
    std::string composerId;
    std::string workId;
    std::vector<std::size_t> sampleIndices;
};

} // namespace

bool CompositionComposerKnowledgePartition::isValid(
    std::size_t sampleCount)
    const noexcept
{
    if (!analysisValid ||
        validationRatio < 0.0 ||
        testRatio < 0.0 ||
        validationRatio >= 1.0 ||
        testRatio >= 1.0 ||
        validationRatio + testRatio >= 1.0)
    {
        return false;
    }

    const auto validate =
        [sampleCount](
            const std::vector<std::size_t>& indices)
        {
            for (std::size_t i = 0;
                 i < indices.size();
                 ++i)
            {
                if (indices[i] >= sampleCount ||
                    (i > 0 &&
                     indices[i - 1] >= indices[i]))
                {
                    return false;
                }
            }

            return true;
        };

    if (!validate(trainingSampleIndices) ||
        !validate(validationSampleIndices) ||
        !validate(testSampleIndices))
    {
        return false;
    }

    std::vector<bool> seen(
        sampleCount,
        false);

    for (const auto* indices :
         {
             &trainingSampleIndices,
             &validationSampleIndices,
             &testSampleIndices
         })
    {
        for (const auto index :
             *indices)
        {
            if (seen[index])
                return false;

            seen[index] = true;
        }
    }

    return std::all_of(
        seen.begin(),
        seen.end(),
        [](bool value)
        {
            return value;
        });
}

std::size_t
CompositionComposerKnowledgePartition::trainingCount()
    const noexcept
{
    return trainingSampleIndices.size();
}

std::size_t
CompositionComposerKnowledgePartition::validationCount()
    const noexcept
{
    return validationSampleIndices.size();
}

std::size_t
CompositionComposerKnowledgePartition::testCount()
    const noexcept
{
    return testSampleIndices.size();
}

bool CompositionComposerKnowledgePartition::contains(
    ComposerKnowledgeSplit split,
    std::size_t sampleIndex)
    const noexcept
{
    const std::vector<std::size_t>* indices = nullptr;

    switch (split)
    {
        case ComposerKnowledgeSplit::Training:
            indices = &trainingSampleIndices;
            break;
        case ComposerKnowledgeSplit::Validation:
            indices = &validationSampleIndices;
            break;
        case ComposerKnowledgeSplit::Test:
            indices = &testSampleIndices;
            break;
    }

    return indices != nullptr &&
           std::binary_search(
               indices->begin(),
               indices->end(),
               sampleIndex);
}

CompositionComposerKnowledgePartition
buildCompositionComposerKnowledgePartition(
    const CompositionComposerKnowledgeCatalog& catalog,
    double validationRatio,
    double testRatio)
    noexcept
{
    CompositionComposerKnowledgePartition result;

    if (!catalog.isValid() ||
        validationRatio < 0.0 ||
        testRatio < 0.0 ||
        validationRatio >= 1.0 ||
        testRatio >= 1.0 ||
        validationRatio + testRatio >= 1.0)
    {
        return result;
    }

    std::vector<
        CompositionComposerKnowledgeSample> samples;

    for (const auto& composer :
         catalog.composers)
    {
        samples.insert(
            samples.end(),
            composer.samples.begin(),
            composer.samples.end());
    }

    if (samples.empty())
        return result;

    std::unordered_map<std::string, std::size_t>
        workLookup;

    std::vector<RankedWork> works;

    for (std::size_t index = 0;
         index < samples.size();
         ++index)
    {
        const auto& metadata =
            samples[index].metadata;

        const auto workId =
            metadata.workId.empty()
                ? metadata.sampleId
                : metadata.workId;

        const auto key =
            metadata.composerId +
            "\x1f" +
            workId;

        const auto iterator =
            workLookup.find(key);

        if (iterator == workLookup.end())
        {
            const auto workIndex =
                works.size();

            workLookup.emplace(
                key,
                workIndex);

            RankedWork work;
            work.hash = stableHash(key);
            work.composerId =
                metadata.composerId;
            work.workId =
                workId;
            work.sampleIndices.push_back(
                index);

            works.push_back(
                std::move(work));
        }
        else
        {
            works[iterator->second]
                .sampleIndices.push_back(index);
        }
    }

    std::sort(
        works.begin(),
        works.end(),
        [](const RankedWork& left,
           const RankedWork& right)
        {
            if (left.composerId !=
                right.composerId)
            {
                return left.composerId <
                       right.composerId;
            }

            if (left.hash != right.hash)
                return left.hash < right.hash;

            return left.workId <
                   right.workId;
        });

    std::size_t cursor = 0;

    while (cursor < works.size())
    {
        const auto composer =
            works[cursor].composerId;

        const auto start =
            cursor;

        while (cursor < works.size() &&
               works[cursor].composerId ==
                   composer)
        {
            ++cursor;
        }

        const auto end = cursor;
        const auto workCount = end - start;

        auto requestedCount =
            [&](double ratio)
            {
                if (ratio <= 0.0)
                    return std::size_t {0};

                return static_cast<std::size_t>(
                    ratio *
                    static_cast<double>(
                        workCount));
            };

        auto validationCount =
            requestedCount(
                validationRatio);

        auto testCount =
            requestedCount(
                testRatio);

        if (workCount >= 3)
        {
            if (validationRatio > 0.0 &&
                validationCount == 0)
            {
                validationCount = 1;
            }

            if (testRatio > 0.0 &&
                testCount == 0)
            {
                testCount = 1;
            }
        }

        while (validationCount +
                   testCount >=
               workCount &&
               validationCount +
                   testCount > 0)
        {
            if (validationCount >= testCount &&
                validationCount > 0)
            {
                --validationCount;
            }
            else if (testCount > 0)
            {
                --testCount;
            }
        }

        for (std::size_t i = 0;
             i < workCount;
             ++i)
        {
            ComposerKnowledgeSplit split =
                ComposerKnowledgeSplit::Training;

            if (i < testCount)
            {
                split =
                    ComposerKnowledgeSplit::Test;
            }
            else if (i <
                     testCount +
                         validationCount)
            {
                split =
                    ComposerKnowledgeSplit::Validation;
            }

            for (const auto sampleIndex :
                 works[start + i].sampleIndices)
            {
                switch (split)
                {
                    case ComposerKnowledgeSplit::Training:
                        result.trainingSampleIndices.push_back(
                            sampleIndex);
                        break;
                    case ComposerKnowledgeSplit::Validation:
                        result.validationSampleIndices.push_back(
                            sampleIndex);
                        break;
                    case ComposerKnowledgeSplit::Test:
                        result.testSampleIndices.push_back(
                            sampleIndex);
                        break;
                }
            }
        }
    }

    std::sort(
        result.trainingSampleIndices.begin(),
        result.trainingSampleIndices.end());

    std::sort(
        result.validationSampleIndices.begin(),
        result.validationSampleIndices.end());

    std::sort(
        result.testSampleIndices.begin(),
        result.testSampleIndices.end());

    result.validationRatio =
        validationRatio;

    result.testRatio =
        testRatio;

    result.analysisValid =
        true;

    return result;
}

} // namespace midigengx::music
