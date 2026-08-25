#include "CompositionDatasetManifest.h"

#include <iomanip>
#include <sstream>

namespace midigengx::music
{
namespace
{

constexpr std::uint64_t kOffsetBasis =
    14695981039346656037ull;

constexpr std::uint64_t kFnvPrime =
    1099511628211ull;

void mixByte(
    std::uint64_t& hash,
    std::uint8_t byte) noexcept
{
    hash ^= byte;
    hash *= kFnvPrime;
}

void mixU64(
    std::uint64_t& hash,
    std::uint64_t value) noexcept
{
    for (int shift = 0;
         shift < 64;
         shift += 8)
    {
        mixByte(
            hash,
            static_cast<std::uint8_t>(
                (value >> shift) &
                0xffull));
    }
}

void mixString(
    std::uint64_t& hash,
    const std::string& value) noexcept
{
    for (const unsigned char character :
         value)
    {
        mixByte(
            hash,
            static_cast<std::uint8_t>(
                character));
    }

    mixByte(hash, 0);
}

std::uint64_t calculateDatasetSignature(
    const CompositionDataset& dataset) noexcept
{
    std::uint64_t hash =
        kOffsetBasis;

    for (const auto& sample :
         dataset.samples)
    {
        mixString(
            hash,
            sample.sampleId);

        mixU64(
            hash,
            sample.globalFeatures.size());

        for (const auto feature :
             sample.globalFeatures)
        {
            const auto* bytes =
                reinterpret_cast<
                    const std::uint8_t*>(
                        &feature);

            for (std::size_t index = 0;
                 index < sizeof(double);
                 ++index)
            {
                mixByte(
                    hash,
                    bytes[index]);
            }
        }

        mixU64(
            hash,
            sample.sectionFeatures.size());

        for (const auto& section :
             sample.sectionFeatures)
        {
            mixU64(
                hash,
                section.size());

            for (const auto feature :
                 section)
            {
                const auto* bytes =
                    reinterpret_cast<
                        const std::uint8_t*>(
                            &feature);

                for (std::size_t index = 0;
                     index < sizeof(double);
                     ++index)
                {
                    mixByte(
                        hash,
                        bytes[index]);
                }
            }
        }
    }

    return hash;
}

} // namespace

bool CompositionDatasetManifest::isValid() const noexcept
{
    if (!analysisValid ||
        schemaVersion !=
            CompositionDatasetSchema::version ||
        globalFeatureWidth !=
            CompositionDatasetSchema::globalFeatureCount ||
        sectionFeatureWidth !=
            CompositionDatasetSchema::sectionFeatureCount)
    {
        return false;
    }

    if (sampleCount == 0)
    {
        return trainingCount == 0 &&
               validationCount == 0 &&
               testCount == 0 &&
               datasetSignature == 0;
    }

    if (trainingCount +
            validationCount +
            testCount !=
        sampleCount)
    {
        return false;
    }

    return datasetSignature != 0;
}

std::string
CompositionDatasetManifest::signatureHex() const
{
    std::ostringstream stream;

    stream << std::hex
           << std::uppercase
           << std::setw(16)
           << std::setfill('0')
           << datasetSignature;

    return stream.str();
}

CompositionDatasetManifest
buildCompositionDatasetManifest(
    const CompositionDataset& dataset,
    const CompositionDatasetQuality& quality,
    const CompositionDatasetPartition& partition) noexcept
{
    CompositionDatasetManifest manifest;

    if (!dataset.isValid() ||
        !quality.isValid() ||
        !partition.isValid(
            dataset.size()) ||
        quality.sampleCount !=
            dataset.size())
    {
        return manifest;
    }

    if (!dataset.samples.empty() &&
        (quality.globalFeatureWidth !=
             CompositionDatasetSchema::globalFeatureCount ||
         quality.sectionFeatureWidth !=
             CompositionDatasetSchema::sectionFeatureCount))
    {
        return manifest;
    }

    manifest.analysisValid = true;

    manifest.schemaVersion =
        CompositionDatasetSchema::version;

    manifest.sampleCount =
        dataset.size();

    manifest.globalFeatureWidth =
        CompositionDatasetSchema::globalFeatureCount;

    manifest.sectionFeatureWidth =
        CompositionDatasetSchema::sectionFeatureCount;

    manifest.trainingCount =
        partition.trainingCount();

    manifest.validationCount =
        partition.validationCount();

    manifest.testCount =
        partition.testCount();

    if (!dataset.samples.empty())
    {
        manifest.datasetSignature =
            calculateDatasetSignature(
                dataset);
    }

    return manifest;
}

} // namespace midigengx::music
