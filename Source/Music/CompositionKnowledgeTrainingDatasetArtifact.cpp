#include "CompositionKnowledgeTrainingDatasetArtifact.h"

#include <cstring>
#include <limits>

namespace midigengx::music
{
namespace
{

constexpr std::size_t headerSize = sizeof(std::uint32_t) * 3;

void writeU32(
    std::vector<std::uint8_t>& bytes,
    std::size_t& offset,
    std::uint32_t value) noexcept
{
    std::memcpy(bytes.data() + offset, &value, sizeof(value));
    offset += sizeof(value);
}

bool readU32(
    const std::vector<std::uint8_t>& bytes,
    std::size_t& offset,
    std::uint32_t& value) noexcept
{
    if (offset + sizeof(value) > bytes.size())
        return false;

    std::memcpy(&value, bytes.data() + offset, sizeof(value));
    offset += sizeof(value);
    return true;
}

bool safeAdd(
    std::size_t left,
    std::size_t right,
    std::size_t& result) noexcept
{
    if (left > std::numeric_limits<std::size_t>::max() - right)
        return false;

    result = left + right;
    return true;
}

} // namespace

bool CompositionKnowledgeTrainingDatasetArtifact::isValid()
    const noexcept
{
    if (bytes.size() < headerSize)
        return false;

    std::size_t offset = 0;
    std::uint32_t magicValue = 0;
    std::uint32_t versionValue = 0;
    std::uint32_t sampleCount = 0;

    if (!readU32(bytes, offset, magicValue) ||
        !readU32(bytes, offset, versionValue) ||
        !readU32(bytes, offset, sampleCount))
    {
        return false;
    }

    if (magicValue != magic ||
        versionValue != version ||
        sampleCount == 0)
    {
        return false;
    }

    constexpr std::size_t sampleHeaderSize =
        sizeof(std::uint32_t) * 2;

    for (std::uint32_t index = 0; index < sampleCount; ++index)
    {
        std::uint32_t idLength = 0;
        std::uint32_t featureCount = 0;

        if (!readU32(bytes, offset, idLength) ||
            idLength == 0 ||
            offset + idLength > bytes.size())
        {
            return false;
        }

        offset += idLength;

        if (!readU32(bytes, offset, featureCount) ||
            featureCount !=
                CompositionKnowledgeTrainingSample::conditioningFeatureCount)
        {
            return false;
        }

        std::size_t featureBytes =
            static_cast<std::size_t>(featureCount) * sizeof(double);

        if (offset + featureBytes + sampleHeaderSize > bytes.size())
            return false;

        // metadata verification flag + sequence event count
        offset += sampleHeaderSize;
        offset += featureBytes;
    }

    return offset == bytes.size();
}

CompositionKnowledgeTrainingDatasetArtifact
serializeCompositionKnowledgeTrainingDataset(
    const CompositionKnowledgeTrainingDataset& dataset) noexcept
{
    CompositionKnowledgeTrainingDatasetArtifact artifact;

    if (!dataset.isValid() ||
        dataset.samples.size() > std::numeric_limits<std::uint32_t>::max())
    {
        return artifact;
    }

    std::size_t totalBytes = headerSize;

    for (const auto& sample : dataset.samples)
    {
        if (sample.sampleId.size() > std::numeric_limits<std::uint32_t>::max())
            return artifact;

        const std::size_t sampleBytes =
            sizeof(std::uint32_t) * 4 +
            sample.sampleId.size() +
            sizeof(double) * sample.conditioningFeatures.size();

        if (!safeAdd(totalBytes, sampleBytes, totalBytes))
            return artifact;
    }

    artifact.bytes.resize(totalBytes);

    std::size_t offset = 0;
    writeU32(artifact.bytes, offset, CompositionKnowledgeTrainingDatasetArtifact::magic);
    writeU32(artifact.bytes, offset, CompositionKnowledgeTrainingDatasetArtifact::version);
    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(dataset.samples.size()));

    for (const auto& sample : dataset.samples)
    {
        writeU32(
            artifact.bytes,
            offset,
            static_cast<std::uint32_t>(sample.sampleId.size()));

        std::memcpy(
            artifact.bytes.data() + offset,
            sample.sampleId.data(),
            sample.sampleId.size());
        offset += sample.sampleId.size();

        writeU32(
            artifact.bytes,
            offset,
            static_cast<std::uint32_t>(sample.conditioningFeatures.size()));

        writeU32(
            artifact.bytes,
            offset,
            sample.metadata.verified ? 1u : 0u);

        writeU32(
            artifact.bytes,
            offset,
            static_cast<std::uint32_t>(sample.sequence.eventCount()));

        std::memcpy(
            artifact.bytes.data() + offset,
            sample.conditioningFeatures.data(),
            sizeof(double) * sample.conditioningFeatures.size());
        offset +=
            sizeof(double) * sample.conditioningFeatures.size();
    }

    if (!artifact.isValid())
        artifact.bytes.clear();

    return artifact;
}

} // namespace midigengx::music
