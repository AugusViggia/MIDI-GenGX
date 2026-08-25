#include "CompositionTrainingCorpusArtifact.h"

#include <cmath>
#include <cstring>
#include <limits>

namespace midigengx::music
{
namespace
{

constexpr std::size_t headerSize =
    sizeof(std::uint32_t) * 8;

void writeU32(
    std::vector<std::uint8_t>& bytes,
    std::size_t& offset,
    std::uint32_t value) noexcept
{
    std::memcpy(
        bytes.data() + offset,
        &value,
        sizeof(value));

    offset +=
        sizeof(value);
}

bool readU32(
    const std::vector<std::uint8_t>& bytes,
    std::size_t& offset,
    std::uint32_t& value) noexcept
{
    if (offset + sizeof(value) >
        bytes.size())
    {
        return false;
    }

    std::memcpy(
        &value,
        bytes.data() + offset,
        sizeof(value));

    offset +=
        sizeof(value);

    return true;
}

void writeDouble(
    std::vector<std::uint8_t>& bytes,
    std::size_t& offset,
    double value) noexcept
{
    std::memcpy(
        bytes.data() + offset,
        &value,
        sizeof(value));

    offset +=
        sizeof(value);
}

bool readDouble(
    const std::vector<std::uint8_t>& bytes,
    std::size_t& offset,
    double& value) noexcept
{
    if (offset + sizeof(value) >
        bytes.size())
    {
        return false;
    }

    std::memcpy(
        &value,
        bytes.data() + offset,
        sizeof(value));

    offset +=
        sizeof(value);

    return true;
}

bool safeMultiply(
    std::size_t a,
    std::size_t b,
    std::size_t& result) noexcept
{
    if (b != 0 &&
        a > std::numeric_limits<std::size_t>::max() / b)
    {
        return false;
    }

    result = a * b;
    return true;
}

bool appendVector(
    std::vector<std::uint8_t>& bytes,
    std::size_t& offset,
    const std::vector<double>& values) noexcept
{
    std::size_t byteCount = 0;

    if (!safeMultiply(
            values.size(),
            sizeof(double),
            byteCount) ||
        offset + byteCount >
            bytes.size())
    {
        return false;
    }

    if (byteCount != 0)
    {
        std::memcpy(
            bytes.data() + offset,
            values.data(),
            byteCount);
    }

    offset += byteCount;
    return true;
}

bool readVector(
    const std::vector<std::uint8_t>& bytes,
    std::size_t& offset,
    std::size_t count,
    std::vector<double>& target) noexcept
{
    std::size_t byteCount = 0;

    if (!safeMultiply(
            count,
            sizeof(double),
            byteCount) ||
        offset + byteCount >
            bytes.size())
    {
        return false;
    }

    target.resize(count);

    if (byteCount != 0)
    {
        std::memcpy(
            target.data(),
            bytes.data() + offset,
            byteCount);
    }

    offset += byteCount;
    return true;
}

bool allFinite(
    const std::vector<double>& values) noexcept
{
    for (const auto value :
         values)
    {
        if (!std::isfinite(value))
            return false;
    }

    return true;
}

std::size_t expectedGlobalCount(
    const CompositionDatasetPreparedView& prepared) noexcept
{
    return prepared.normalizedBatch.globalMatrix.size();
}

std::size_t expectedSectionCount(
    const CompositionDatasetPreparedView& prepared) noexcept
{
    return prepared.normalizedBatch.sectionMatrix.size();
}

} // namespace

bool CompositionTrainingCorpusArtifact::isValid()
    const noexcept
{
    if (bytes.size() < headerSize)
        return false;

    std::size_t offset = 0;

    std::uint32_t artifactMagic = 0;
    std::uint32_t artifactVersion = 0;
    std::uint32_t schemaVersion = 0;
    std::uint32_t batchVersion = 0;
    std::uint32_t sampleCount = 0;
    std::uint32_t maxSectionCount = 0;
    std::uint32_t globalCount = 0;
    std::uint32_t sectionCount = 0;

    if (!readU32(bytes, offset, artifactMagic) ||
        !readU32(bytes, offset, artifactVersion) ||
        !readU32(bytes, offset, schemaVersion) ||
        !readU32(bytes, offset, batchVersion) ||
        !readU32(bytes, offset, sampleCount) ||
        !readU32(bytes, offset, maxSectionCount) ||
        !readU32(bytes, offset, globalCount) ||
        !readU32(bytes, offset, sectionCount))
    {
        return false;
    }

    if (artifactMagic !=
            CompositionTrainingCorpusArtifact::magic ||
        artifactVersion !=
            CompositionTrainingCorpusArtifact::version ||
        schemaVersion != CompositionDataset::schemaVersion ||
        batchVersion != CompositionDatasetBatch::batchVersion)
    {
        return false;
    }

    std::size_t scalarCount = 0;
    std::size_t sectionMaskCount = 0;

    if (!safeMultiply(
            static_cast<std::size_t>(sampleCount),
            static_cast<std::size_t>(maxSectionCount),
            sectionMaskCount))
    {
        return false;
    }

    if (globalCount !=
            static_cast<std::uint32_t>(
                static_cast<std::size_t>(sampleCount) *
                static_cast<std::size_t>(
                    CompositionDatasetSchema::globalFeatureCount)) ||
        sectionCount !=
            static_cast<std::uint32_t>(
                sectionMaskCount *
                static_cast<std::size_t>(
                    CompositionDatasetSchema::sectionFeatureCount)))
    {
        return false;
    }

    scalarCount =
        static_cast<std::size_t>(globalCount) +
        static_cast<std::size_t>(sectionCount) +
        sectionMaskCount;

    std::size_t payloadBytes = 0;

    if (!safeMultiply(
            scalarCount,
            sizeof(double),
            payloadBytes))
    {
        return false;
    }

    return offset + payloadBytes ==
           bytes.size();
}

CompositionTrainingCorpusArtifact
serializeCompositionTrainingCorpus(
    const CompositionDatasetPreparedView& prepared)
    noexcept
{
    CompositionTrainingCorpusArtifact artifact;

    if (!prepared.isValid())
        return artifact;

    const auto& batch =
        prepared.normalizedBatch;

    const auto sampleCount =
        batch.sampleCount;

    const auto maxSectionCount =
        batch.maxSectionCount;

    const auto globalCount =
        batch.globalMatrix.size();

    const auto sectionCount =
        batch.sectionMatrix.size();

    const auto maskCount =
        batch.sectionMask.size();

    if (!allFinite(batch.globalMatrix) ||
        !allFinite(batch.sectionMatrix) ||
        !allFinite(batch.sectionMask))
    {
        return artifact;
    }

    if (globalCount >
            std::numeric_limits<std::uint32_t>::max() ||
        sectionCount >
            std::numeric_limits<std::uint32_t>::max() ||
        sampleCount >
            std::numeric_limits<std::uint32_t>::max() ||
        maxSectionCount >
            std::numeric_limits<std::uint32_t>::max())
    {
        return artifact;
    }

    const auto scalarCount =
        globalCount +
        sectionCount +
        maskCount;

    std::size_t payloadBytes = 0;

    if (!safeMultiply(
            scalarCount,
            sizeof(double),
            payloadBytes) ||
        payloadBytes >
            std::numeric_limits<std::size_t>::max() -
                headerSize)
    {
        return artifact;
    }

    artifact.bytes.resize(
        headerSize +
        payloadBytes);

    std::size_t offset = 0;

    writeU32(
        artifact.bytes,
        offset,
        CompositionTrainingCorpusArtifact::magic);

    writeU32(
        artifact.bytes,
        offset,
        CompositionTrainingCorpusArtifact::version);

    writeU32(
        artifact.bytes,
        offset,
        CompositionDataset::schemaVersion);

    writeU32(
        artifact.bytes,
        offset,
        CompositionDatasetBatch::batchVersion);

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            sampleCount));

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            maxSectionCount));

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            globalCount));

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            sectionCount));

    if (!appendVector(
            artifact.bytes,
            offset,
            batch.globalMatrix) ||
        !appendVector(
            artifact.bytes,
            offset,
            batch.sectionMatrix) ||
        !appendVector(
            artifact.bytes,
            offset,
            batch.sectionMask))
    {
        artifact.bytes.clear();
    }

    return artifact;
}

bool deserializeCompositionTrainingCorpus(
    const CompositionTrainingCorpusArtifact& artifact,
    CompositionDatasetPreparedView& prepared)
    noexcept
{
    if (!artifact.isValid())
        return false;

    std::size_t offset = 0;

    std::uint32_t artifactMagic = 0;
    std::uint32_t artifactVersion = 0;
    std::uint32_t schemaVersion = 0;
    std::uint32_t batchVersion = 0;
    std::uint32_t sampleCount = 0;
    std::uint32_t maxSectionCount = 0;
    std::uint32_t globalCount = 0;
    std::uint32_t sectionCount = 0;

    if (!readU32(artifact.bytes, offset, artifactMagic) ||
        !readU32(artifact.bytes, offset, artifactVersion) ||
        !readU32(artifact.bytes, offset, schemaVersion) ||
        !readU32(artifact.bytes, offset, batchVersion) ||
        !readU32(artifact.bytes, offset, sampleCount) ||
        !readU32(artifact.bytes, offset, maxSectionCount) ||
        !readU32(artifact.bytes, offset, globalCount) ||
        !readU32(artifact.bytes, offset, sectionCount))
    {
        return false;
    }

    if (artifactMagic !=
            CompositionTrainingCorpusArtifact::magic ||
        artifactVersion !=
            CompositionTrainingCorpusArtifact::version ||
        schemaVersion != CompositionDataset::schemaVersion ||
        batchVersion != CompositionDatasetBatch::batchVersion)
    {
        return false;
    }

    CompositionDatasetPreparedView candidate;

    auto& batch =
        candidate.normalizedBatch;

    batch.sampleCount =
        sampleCount;

    batch.maxSectionCount =
        maxSectionCount;

    batch.globalFeatureWidth =
        CompositionDatasetSchema::globalFeatureCount;

    batch.sectionFeatureWidth =
        CompositionDatasetSchema::sectionFeatureCount;

    const std::size_t globalSize =
        static_cast<std::size_t>(
            globalCount);

    const std::size_t sectionSize =
        static_cast<std::size_t>(
            sectionCount);

    std::size_t maskSize = 0;

    if (!safeMultiply(
            static_cast<std::size_t>(sampleCount),
            static_cast<std::size_t>(maxSectionCount),
            maskSize))
    {
        return false;
    }

    if (globalSize !=
            batch.sampleCount *
            batch.globalFeatureWidth ||
        sectionSize !=
            maskSize *
            batch.sectionFeatureWidth)
    {
        return false;
    }

    if (!readVector(
            artifact.bytes,
            offset,
            globalSize,
            batch.globalMatrix) ||
        !readVector(
            artifact.bytes,
            offset,
            sectionSize,
            batch.sectionMatrix) ||
        !readVector(
            artifact.bytes,
            offset,
            maskSize,
            batch.sectionMask))
    {
        return false;
    }

    if (offset != artifact.bytes.size() ||
        !allFinite(batch.globalMatrix) ||
        !allFinite(batch.sectionMatrix) ||
        !allFinite(batch.sectionMask))
    {
        return false;
    }

    batch.analysisValid =
        true;

    candidate.analysisValid =
        true;

    // The artifact intentionally stores only the normalized learning matrices.
    // Partition/normalization provenance must be supplied by the real corpus
    // builder when it creates the prepared view used for training.
    candidate.partition =
        CompositionDatasetPartition{};

    candidate.normalization =
        CompositionDatasetNormalization{};

    prepared =
        std::move(candidate);

    return prepared.normalizedBatch.isValid() &&
           prepared.analysisValid;
}

} // namespace midigengx::music
