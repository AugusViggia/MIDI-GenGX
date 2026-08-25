#include "CompositionDatasetPartition.h"

#include <cstdint>

namespace midigengx::music
{
namespace
{

std::uint64_t stableHash(
    const std::string& value) noexcept
{
    // FNV-1a with fixed-width constants gives the partitioner a stable
    // cross-run and cross-machine mapping for the same sample id.
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

double hashUnit(
    const std::string& value) noexcept
{
    constexpr double denominator =
        18446744073709551615.0;

    return static_cast<double>(
               stableHash(value)) /
           denominator;
}

} // namespace

bool CompositionDatasetPartition::isValid(
    std::size_t datasetSize) const noexcept
{
    if (!analysisValid ||
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

    const auto validateIndices =
        [datasetSize](const std::vector<std::size_t>& indices)
        {
            std::size_t previous = 0;
            bool first = true;

            for (const auto index :
                 indices)
            {
                if (index >= datasetSize)
                    return false;

                if (!first &&
                    index <= previous)
                {
                    return false;
                }

                previous = index;
                first = false;
            }

            return true;
        };

    if (!validateIndices(trainingIndices) ||
        !validateIndices(validationIndices) ||
        !validateIndices(testIndices))
    {
        return false;
    }

    std::vector<bool> seen(
        datasetSize,
        false);

    const auto mark =
        [&seen](const std::vector<std::size_t>& indices)
        {
            for (const auto index :
                 indices)
            {
                if (seen[index])
                    return false;

                seen[index] = true;
            }

            return true;
        };

    if (!mark(trainingIndices) ||
        !mark(validationIndices) ||
        !mark(testIndices))
    {
        return false;
    }

    std::size_t covered = 0;

    for (const auto present :
         seen)
    {
        if (present)
            ++covered;
    }

    return covered == datasetSize;
}

std::size_t CompositionDatasetPartition::trainingCount() const noexcept
{
    return trainingIndices.size();
}

std::size_t CompositionDatasetPartition::validationCount() const noexcept
{
    return validationIndices.size();
}

std::size_t CompositionDatasetPartition::testCount() const noexcept
{
    return testIndices.size();
}

CompositionDatasetPartition
buildCompositionDatasetPartition(
    const CompositionDataset& dataset,
    double validationRatio,
    double testRatio) noexcept
{
    CompositionDatasetPartition partition;

    if (!dataset.isValid() ||
        validationRatio < 0.0 ||
        testRatio < 0.0 ||
        validationRatio >= 1.0 ||
        testRatio >= 1.0 ||
        validationRatio +
                testRatio >=
            1.0)
    {
        return partition;
    }

    partition.analysisValid = true;
    partition.validationRatio =
        validationRatio;
    partition.testRatio =
        testRatio;

    const auto sampleCount =
        dataset.samples.size();

    if (sampleCount == 0)
        return partition;

    struct RankedSample
    {
        std::uint64_t hash = 0;
        std::size_t index = 0;
    };

    std::vector<RankedSample> rankedSamples;
    rankedSamples.reserve(
        sampleCount);

    for (std::size_t index = 0;
         index < sampleCount;
         ++index)
    {
        rankedSamples.push_back(
        {
            stableHash(
                dataset.samples[index]
                    .sampleId),
            index
        });
    }

    std::sort(
        rankedSamples.begin(),
        rankedSamples.end(),
        [](const RankedSample& a,
           const RankedSample& b)
        {
            if (a.hash != b.hash)
                return a.hash < b.hash;

            return a.index < b.index;
        });

    const auto requestedCount =
        [sampleCount](double ratio) -> std::size_t
        {
            if (ratio <= 0.0 ||
                sampleCount == 0)
            {
                return 0;
            }

            return static_cast<std::size_t>(
                ratio *
                static_cast<double>(
                    sampleCount));
        };

    std::size_t requestedValidation =
        requestedCount(
            validationRatio);

    std::size_t requestedTest =
        requestedCount(
            testRatio);

    const std::size_t activeSplits =
        static_cast<std::size_t>(
            validationRatio > 0.0) +
        static_cast<std::size_t>(
            testRatio > 0.0) +
        static_cast<std::size_t>(
            validationRatio +
                testRatio <
            1.0);

    // When enough samples exist, every explicitly requested split receives
    // at least one sample. This prevents a perfectly valid small dataset from
    // accidentally losing its validation or test set solely because a hash
    // happened to fall outside a probability bucket.
    if (sampleCount >= activeSplits)
    {
        if (validationRatio > 0.0 &&
            requestedValidation == 0)
        {
            requestedValidation = 1;
        }

        if (testRatio > 0.0 &&
            requestedTest == 0)
        {
            requestedTest = 1;
        }

        const auto maxNonTraining =
            sampleCount - 1;

        if (requestedValidation +
                requestedTest >
            maxNonTraining)
        {
            while (requestedValidation +
                       requestedTest >
                   maxNonTraining)
            {
                if (requestedValidation >=
                    requestedTest &&
                    requestedValidation > 0)
                {
                    --requestedValidation;
                }
                else if (requestedTest > 0)
                {
                    --requestedTest;
                }
                else
                {
                    break;
                }
            }
        }
    }

    const auto trainingCount =
        sampleCount -
        requestedValidation -
        requestedTest;

    std::size_t cursor = 0;

    for (std::size_t i = 0;
         i < requestedTest;
         ++i)
    {
        partition.testIndices.push_back(
            rankedSamples[cursor++].index);
    }

    for (std::size_t i = 0;
         i < requestedValidation;
         ++i)
    {
        partition.validationIndices.push_back(
            rankedSamples[cursor++].index);
    }

    for (std::size_t i = 0;
         i < trainingCount;
         ++i)
    {
        partition.trainingIndices.push_back(
            rankedSamples[cursor++].index);
    }

    const auto sortIndices =
        [](std::vector<std::size_t>& indices)
        {
            std::sort(
                indices.begin(),
                indices.end());
        };

    sortIndices(
        partition.trainingIndices);

    sortIndices(
        partition.validationIndices);

    sortIndices(
        partition.testIndices);

    return partition;
}

} // namespace midigengx::music
