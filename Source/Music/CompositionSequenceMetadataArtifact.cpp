#include "CompositionSequenceMetadataArtifact.h"

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

    result =
        left + right;

    return true;
}

bool writeString(
    std::vector<std::uint8_t>& bytes,
    std::size_t& offset,
    const std::string& value) noexcept
{
    if (value.size() >
        std::numeric_limits<std::uint32_t>::max())
    {
        return false;
    }

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

} // namespace

bool CompositionSequenceMetadataArtifact::isValid()
    const noexcept
{
    if (bytes.size() < headerSize)
        return false;

    std::size_t offset = 0;

    std::uint32_t artifactMagic = 0;
    std::uint32_t artifactVersion = 0;
    std::uint32_t entryCount = 0;
    std::uint32_t metadataVersion = 0;

    if (!readU32(
            bytes,
            offset,
            artifactMagic) ||
        !readU32(
            bytes,
            offset,
            artifactVersion) ||
        !readU32(
            bytes,
            offset,
            entryCount) ||
        !readU32(
            bytes,
            offset,
            metadataVersion))
    {
        return false;
    }

    if (artifactMagic !=
            CompositionSequenceMetadataArtifact::magic ||
        artifactVersion !=
            CompositionSequenceMetadataArtifact::version ||
        metadataVersion !=
            CompositionSequenceMetadata::version ||
        entryCount == 0)
    {
        return false;
    }

    for (std::uint32_t index = 0;
         index < entryCount;
         ++index)
    {
        for (int field = 0;
             field < 7;
             ++field)
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

        if (offset + sizeof(std::uint8_t) >
            bytes.size())
        {
            return false;
        }

        offset += sizeof(std::uint8_t);
    }

    return offset == bytes.size();
}

CompositionSequenceMetadataArtifact
serializeCompositionSequenceMetadataCatalog(
    const CompositionSequenceMetadataCatalog& catalog)
    noexcept
{
    CompositionSequenceMetadataArtifact artifact;

    if (!catalog.isValid() ||
        catalog.entries.size() >
            std::numeric_limits<std::uint32_t>::max())
    {
        return artifact;
    }

    std::size_t totalBytes =
        headerSize;

    for (const auto& entry :
         catalog.entries)
    {
        const std::string fields[] =
        {
            entry.sampleId,
            entry.composerId,
            entry.workId,
            entry.movementId,
            entry.styleId,
            entry.eraId
        };

        // Instrumentation is stored in the same field group.
        const std::string* allFields[] =
        {
            &fields[0], &fields[1], &fields[2],
            &fields[3], &fields[4], &fields[5],
            &entry.instrumentationId
        };

        for (const auto* value :
             allFields)
        {
            if (value->size() >
                std::numeric_limits<std::uint32_t>::max())
            {
                return artifact;
            }

            std::size_t next = 0;

            if (!safeAdd(
                    totalBytes,
                    sizeof(std::uint32_t),
                    next) ||
                !safeAdd(
                    next,
                    value->size(),
                    totalBytes))
            {
                return artifact;
            }
        }

        if (!safeAdd(
                totalBytes,
                sizeof(std::uint8_t),
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
        CompositionSequenceMetadataArtifact::magic);

    writeU32(
        artifact.bytes,
        offset,
        CompositionSequenceMetadataArtifact::version);

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            catalog.entries.size()));

    writeU32(
        artifact.bytes,
        offset,
        CompositionSequenceMetadata::version);

    for (const auto& entry :
         catalog.entries)
    {
        const std::string fields[] =
        {
            entry.sampleId,
            entry.composerId,
            entry.workId,
            entry.movementId,
            entry.styleId,
            entry.eraId,
            entry.instrumentationId
        };

        for (const auto& field :
             fields)
        {
            if (!writeString(
                    artifact.bytes,
                    offset,
                    field))
            {
                artifact.bytes.clear();
                return artifact;
            }
        }

        artifact.bytes[offset++] =
            entry.verified
                ? 1
                : 0;
    }

    return artifact;
}

bool deserializeCompositionSequenceMetadataCatalog(
    const CompositionSequenceMetadataArtifact& artifact,
    CompositionSequenceMetadataCatalog& catalog)
    noexcept
{
    catalog =
        CompositionSequenceMetadataCatalog{};

    if (!artifact.isValid())
        return false;

    std::size_t offset = 0;

    std::uint32_t artifactMagic = 0;
    std::uint32_t artifactVersion = 0;
    std::uint32_t entryCount = 0;
    std::uint32_t metadataVersion = 0;

    if (!readU32(
            artifact.bytes,
            offset,
            artifactMagic) ||
        !readU32(
            artifact.bytes,
            offset,
            artifactVersion) ||
        !readU32(
            artifact.bytes,
            offset,
            entryCount) ||
        !readU32(
            artifact.bytes,
            offset,
            metadataVersion))
    {
        return false;
    }

    if (artifactMagic !=
            CompositionSequenceMetadataArtifact::magic ||
        artifactVersion !=
            CompositionSequenceMetadataArtifact::version ||
        metadataVersion !=
            CompositionSequenceMetadata::version)
    {
        return false;
    }

    std::vector<CompositionSequenceMetadata> entries;
    entries.reserve(
        entryCount);

    for (std::uint32_t index = 0;
         index < entryCount;
         ++index)
    {
        CompositionSequenceMetadata entry;

        std::string* fields[] =
        {
            &entry.sampleId,
            &entry.composerId,
            &entry.workId,
            &entry.movementId,
            &entry.styleId,
            &entry.eraId,
            &entry.instrumentationId
        };

        for (auto* field :
             fields)
        {
            if (!readString(
                    artifact.bytes,
                    offset,
                    *field))
            {
                return false;
            }
        }

        if (offset + sizeof(std::uint8_t) >
            artifact.bytes.size())
        {
            return false;
        }

        entry.verified =
            artifact.bytes[offset++] != 0;

        entry.valid =
            true;

        if (!entry.isValid())
            return false;

        entries.push_back(
            std::move(entry));
    }

    if (offset !=
        artifact.bytes.size())
    {
        return false;
    }

    catalog =
        buildCompositionSequenceMetadataCatalog(
            entries);

    return catalog.isValid();
}

} // namespace midigengx::music
