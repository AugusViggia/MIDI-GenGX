#include "CompositionConditionedTrainingDatasetArtifact.h"

#include <cstring>
#include <limits>
#include <string>

namespace midigengx::music
{
namespace
{

constexpr std::size_t headerSize =
    sizeof(std::uint32_t) * 4;

void writeU32(
    std::vector<std::uint8_t>& bytes,
    std::size_t& offset,
    std::uint32_t value) noexcept
{
    std::memcpy(
        bytes.data() + offset,
        &value,
        sizeof(value));
    offset += sizeof(value);
}

bool readU32(
    const std::vector<std::uint8_t>& bytes,
    std::size_t& offset,
    std::uint32_t& value) noexcept
{
    if (offset + sizeof(value) > bytes.size())
        return false;

    std::memcpy(
        &value,
        bytes.data() + offset,
        sizeof(value));
    offset += sizeof(value);
    return true;
}

bool writeString(
    std::vector<std::uint8_t>& bytes,
    std::size_t& offset,
    const std::string& value) noexcept
{
    if (value.size() >
        std::numeric_limits<std::uint32_t>::max())
        return false;

    writeU32(
        bytes,
        offset,
        static_cast<std::uint32_t>(
            value.size()));

    if (!value.empty())
    {
        std::memcpy(
            bytes.data() + offset,
            value.data(),
            value.size());

        offset += value.size();
    }

    return true;
}

bool readString(
    const std::vector<std::uint8_t>& bytes,
    std::size_t& offset,
    std::string& value) noexcept
{
    std::uint32_t size = 0;

    if (!readU32(
            bytes,
            offset,
            size) ||
        offset + size > bytes.size())
    {
        return false;
    }

    value.assign(
        reinterpret_cast<const char*>(
            bytes.data() + offset),
        size);

    offset += size;
    return true;
}

bool safeAdd(
    std::size_t left,
    std::size_t right,
    std::size_t& result) noexcept
{
    if (left >
        std::numeric_limits<std::size_t>::max() -
            right)
    {
        return false;
    }

    result = left + right;
    return true;
}

} // namespace

bool CompositionConditionedTrainingDatasetArtifact::isValid()
    const noexcept
{
    if (bytes.size() < headerSize)
        return false;

    std::size_t offset = 0;

    std::uint32_t magicValue = 0;
    std::uint32_t versionValue = 0;
    std::uint32_t composerCount = 0;
    std::uint32_t styleCount = 0;

    if (!readU32(bytes, offset, magicValue) ||
        !readU32(bytes, offset, versionValue) ||
        !readU32(bytes, offset, composerCount) ||
        !readU32(bytes, offset, styleCount))
    {
        return false;
    }

    if (magicValue !=
            CompositionConditionedTrainingDatasetArtifact::magic ||
        versionValue !=
            CompositionConditionedTrainingDatasetArtifact::version ||
        composerCount == 0 ||
        styleCount == 0)
    {
        return false;
    }

    // The artifact is a deterministic condition manifest. It stores all
    // conditioning vocabulary and sample -> category IDs. The canonical note
    // sequences remain in CompositionMidiTrainingCorpusArtifact.
    for (std::uint32_t group = 0; group < 4; ++group)
    {
        std::uint32_t count = 0;
        if (group == 0)
            count = composerCount;
        else if (group == 1)
            count = styleCount;
        else
        {
            if (!readU32(
                    bytes,
                    offset,
                    count))
                return false;
        }

        for (std::uint32_t index = 0;
             index < count;
             ++index)
        {
            std::string value;
            if (!readString(
                    bytes,
                    offset,
                    value) ||
                value.empty())
            {
                return false;
            }
        }
    }

    std::uint32_t sampleCount = 0;

    if (!readU32(
            bytes,
            offset,
            sampleCount) ||
        sampleCount == 0)
    {
        return false;
    }

    for (std::uint32_t index = 0;
         index < sampleCount;
         ++index)
    {
        std::string sampleId;

        if (!readString(
                bytes,
                offset,
                sampleId) ||
            sampleId.empty() ||
            offset +
                sizeof(std::uint32_t) * 4 >
                bytes.size())
        {
            return false;
        }

        offset +=
            sizeof(std::uint32_t) * 4;
    }

    return offset ==
           bytes.size();
}

CompositionConditionedTrainingDatasetArtifact
serializeCompositionConditionedTrainingDataset(
    const CompositionConditionedTrainingDataset& dataset)
    noexcept
{
    CompositionConditionedTrainingDatasetArtifact artifact;

    if (!dataset.isValid())
        return artifact;

    const auto vocabulary = &dataset.vocabulary;

    std::size_t totalBytes =
        headerSize;

    const std::vector<std::string>* groups[] =
    {
        &vocabulary->composers,
        &vocabulary->styles,
        &vocabulary->eras,
        &vocabulary->instrumentations
    };

    for (std::size_t groupIndex = 0;
         groupIndex < 4;
         ++groupIndex)
    {
        const auto* group =
            groups[groupIndex];

        if (group->size() >
            std::numeric_limits<std::uint32_t>::max())
        {
            return artifact;
        }

        // Composer/style counts are already in the fixed header.
        // Era/instrumentation counts are serialized before their groups.
        if (groupIndex >= 2)
        {
            if (!safeAdd(
                    totalBytes,
                    sizeof(std::uint32_t),
                    totalBytes))
            {
                return artifact;
            }
        }

        for (const auto& value :
             *group)
        {
            if (!safeAdd(
                    totalBytes,
                    sizeof(std::uint32_t) +
                    value.size(),
                    totalBytes))
            {
                return artifact;
            }
        }
    }

    if (dataset.samples.size() >
        std::numeric_limits<std::uint32_t>::max())
        return artifact;

    if (!safeAdd(
            totalBytes,
            sizeof(std::uint32_t),
            totalBytes))
        return artifact;

    for (const auto& sample :
         dataset.samples)
    {
        if (!safeAdd(
                totalBytes,
                sizeof(std::uint32_t) +
                sample.sequence.sampleId.size() +
                sizeof(std::uint32_t) * 4,
                totalBytes))
        {
            return artifact;
        }
    }

    artifact.bytes.resize(
        totalBytes);

    std::size_t offset = 0;

    writeU32(
        artifact.bytes,
        offset,
        CompositionConditionedTrainingDatasetArtifact::magic);

    writeU32(
        artifact.bytes,
        offset,
        CompositionConditionedTrainingDatasetArtifact::version);

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            vocabulary->composers.size()));

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            vocabulary->styles.size()));

    for (std::size_t groupIndex = 0;
         groupIndex < 4;
         ++groupIndex)
    {
        const auto* group =
            groups[groupIndex];

        // Composer/style counts are stored in the fixed header.
        // Era/instrumentation counts are stored immediately before their
        // corresponding vocabularies.
        if (groupIndex >= 2)
        {
            writeU32(
                artifact.bytes,
                offset,
                static_cast<std::uint32_t>(
                    group->size()));
        }

        for (const auto& value :
             *group)
        {
            if (!writeString(
                    artifact.bytes,
                    offset,
                    value))
            {
                artifact.bytes.clear();
                return artifact;
            }
        }
    }

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            dataset.samples.size()));

    for (const auto& sample :
         dataset.samples)
    {
        if (!writeString(
                artifact.bytes,
                offset,
                sample.sequence.sampleId))
        {
            artifact.bytes.clear();
            return artifact;
        }

        writeU32(
            artifact.bytes,
            offset,
            sample.composerIndex);

        writeU32(
            artifact.bytes,
            offset,
            sample.styleIndex);

        writeU32(
            artifact.bytes,
            offset,
            sample.eraIndex);

        writeU32(
            artifact.bytes,
            offset,
            sample.instrumentationIndex);
    }

    return artifact;
}

bool deserializeCompositionConditionedTrainingDataset(
    const CompositionConditionedTrainingDatasetArtifact& artifact,
    CompositionConditionedTrainingDataset& dataset)
    noexcept
{
    dataset =
        CompositionConditionedTrainingDataset{};

    if (!artifact.isValid())
        return false;

    // The condition manifest alone is intentionally not sufficient to recreate
    // note sequences. The full conditioned dataset object must be rebuilt from
    // the canonical sequence corpus plus metadata catalog.
    return false;
}

} // namespace midigengx::music
