#include "CompositionMidiTrainingCorpusArtifact.h"

#include <cstring>
#include <limits>

namespace midigengx::music
{
namespace
{

constexpr std::size_t headerSize =
    sizeof(std::uint32_t) * 5;

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

void writeBytes(
    std::vector<std::uint8_t>& bytes,
    std::size_t& offset,
    const void* data,
    std::size_t size) noexcept
{
    if (size == 0)
        return;

    std::memcpy(
        bytes.data() + offset,
        data,
        size);

    offset += size;
}

bool readBytes(
    const std::vector<std::uint8_t>& bytes,
    std::size_t& offset,
    void* data,
    std::size_t size) noexcept
{
    if (offset + size > bytes.size())
        return false;

    if (size != 0)
    {
        std::memcpy(
            data,
            bytes.data() + offset,
            size);
    }

    offset += size;
    return true;
}

bool safeAdd(
    std::size_t a,
    std::size_t b,
    std::size_t& result) noexcept
{
    if (a > std::numeric_limits<std::size_t>::max() - b)
        return false;

    result = a + b;
    return true;
}

bool safeMultiply(
    std::size_t a,
    std::size_t b,
    std::size_t& result) noexcept
{
    if (b != 0 &&
        a > std::numeric_limits<std::size_t>::max() / b)
        return false;

    result = a * b;
    return true;
}

} // namespace

bool CompositionMidiTrainingCorpusArtifact::isValid()
    const noexcept
{
    if (bytes.size() < headerSize)
        return false;

    std::size_t offset = 0;

    std::uint32_t artifactMagic = 0;
    std::uint32_t artifactVersion = 0;
    std::uint32_t sequenceCount = 0;
    std::uint32_t featureWidth = 0;
    std::uint32_t totalEventCount = 0;

    if (!readU32(bytes, offset, artifactMagic) ||
        !readU32(bytes, offset, artifactVersion) ||
        !readU32(bytes, offset, sequenceCount) ||
        !readU32(bytes, offset, featureWidth) ||
        !readU32(bytes, offset, totalEventCount))
    {
        return false;
    }

    if (artifactMagic !=
            CompositionMidiTrainingCorpusArtifact::magic ||
        artifactVersion !=
            CompositionMidiTrainingCorpusArtifact::version ||
        featureWidth !=
            CompositionMidiTrainingEvent::featureCount ||
        sequenceCount == 0 ||
        totalEventCount == 0)
    {
        return false;
    }

    for (std::uint32_t sequenceIndex = 0;
         sequenceIndex < sequenceCount;
         ++sequenceIndex)
    {
        std::uint32_t idLength = 0;
        std::uint32_t eventCount = 0;

        if (!readU32(bytes, offset, idLength) ||
            !readU32(bytes, offset, eventCount) ||
            idLength == 0 ||
            eventCount == 0)
        {
            return false;
        }

        if (offset + idLength > bytes.size())
            return false;

        offset += idLength;

        std::size_t scalarCount = 0;
        std::size_t byteCount = 0;

        if (!safeMultiply(
                static_cast<std::size_t>(eventCount),
                static_cast<std::size_t>(featureWidth),
                scalarCount) ||
            !safeMultiply(
                scalarCount,
                sizeof(double),
                byteCount) ||
            offset + byteCount > bytes.size())
        {
            return false;
        }

        offset += byteCount;
    }

    return offset == bytes.size();
}

CompositionMidiTrainingCorpusArtifact
serializeCompositionMidiTrainingSequences(
    const std::vector<CompositionMidiTrainingSequence>& sequences)
    noexcept
{
    CompositionMidiTrainingCorpusArtifact artifact;

    if (sequences.empty())
        return artifact;

    std::size_t totalBytes = headerSize;
    std::size_t totalEventCount = 0;

    for (const auto& sequence :
         sequences)
    {
        if (!sequence.isValid())
            return artifact;

        if (sequence.featureWidth !=
            CompositionMidiTrainingEvent::featureCount ||
            sequence.sampleId.empty())
        {
            return artifact;
        }

        std::size_t byteCount = 0;

        if (!safeMultiply(
                sequence.events.size(),
                sequence.featureWidth,
                byteCount) ||
            !safeMultiply(
                byteCount,
                sizeof(double),
                byteCount))
        {
            return artifact;
        }

        if (sequence.sampleId.size() >
                std::numeric_limits<std::uint32_t>::max() ||
            sequence.events.size() >
                std::numeric_limits<std::uint32_t>::max())
        {
            return artifact;
        }

        if (!safeAdd(
                totalBytes,
                sizeof(std::uint32_t) * 2,
                totalBytes) ||
            !safeAdd(
                totalBytes,
                sequence.sampleId.size(),
                totalBytes) ||
            !safeAdd(
                totalBytes,
                byteCount,
                totalBytes) ||
            !safeAdd(
                totalEventCount,
                sequence.events.size(),
                totalEventCount))
        {
            return artifact;
        }
    }

    if (sequences.size() >
            std::numeric_limits<std::uint32_t>::max() ||
        totalEventCount >
            std::numeric_limits<std::uint32_t>::max())
    {
        return artifact;
    }

    artifact.bytes.resize(
        totalBytes);

    std::size_t offset = 0;

    writeU32(
        artifact.bytes,
        offset,
        CompositionMidiTrainingCorpusArtifact::magic);

    writeU32(
        artifact.bytes,
        offset,
        CompositionMidiTrainingCorpusArtifact::version);

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            sequences.size()));

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            CompositionMidiTrainingEvent::featureCount));

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            totalEventCount));

    for (const auto& sequence :
         sequences)
    {
        writeU32(
            artifact.bytes,
            offset,
            static_cast<std::uint32_t>(
                sequence.sampleId.size()));

        writeU32(
            artifact.bytes,
            offset,
            static_cast<std::uint32_t>(
                sequence.events.size()));

        writeBytes(
            artifact.bytes,
            offset,
            sequence.sampleId.data(),
            sequence.sampleId.size());

        for (const auto& event :
             sequence.events)
        {
            writeBytes(
                artifact.bytes,
                offset,
                event.features.data(),
                event.features.size() *
                    sizeof(double));
        }
    }

    return artifact;
}

bool deserializeCompositionMidiTrainingSequences(
    const CompositionMidiTrainingCorpusArtifact& artifact,
    std::vector<CompositionMidiTrainingSequence>& sequences)
    noexcept
{
    sequences.clear();

    if (!artifact.isValid())
        return false;

    std::size_t offset = 0;

    std::uint32_t artifactMagic = 0;
    std::uint32_t artifactVersion = 0;
    std::uint32_t sequenceCount = 0;
    std::uint32_t featureWidth = 0;
    std::uint32_t totalEventCount = 0;

    if (!readU32(artifact.bytes, offset, artifactMagic) ||
        !readU32(artifact.bytes, offset, artifactVersion) ||
        !readU32(artifact.bytes, offset, sequenceCount) ||
        !readU32(artifact.bytes, offset, featureWidth) ||
        !readU32(artifact.bytes, offset, totalEventCount))
    {
        return false;
    }

    if (artifactMagic !=
            CompositionMidiTrainingCorpusArtifact::magic ||
        artifactVersion !=
            CompositionMidiTrainingCorpusArtifact::version ||
        featureWidth !=
            CompositionMidiTrainingEvent::featureCount)
    {
        return false;
    }

    sequences.reserve(sequenceCount);

    std::size_t decodedEventCount = 0;

    for (std::uint32_t sequenceIndex = 0;
         sequenceIndex < sequenceCount;
         ++sequenceIndex)
    {
        std::uint32_t idLength = 0;
        std::uint32_t eventCount = 0;

        if (!readU32(
                artifact.bytes,
                offset,
                idLength) ||
            !readU32(
                artifact.bytes,
                offset,
                eventCount) ||
            idLength == 0 ||
            eventCount == 0)
        {
            sequences.clear();
            return false;
        }

        CompositionMidiTrainingSequence sequence;

        sequence.sampleId.resize(
            idLength);

        if (!readBytes(
                artifact.bytes,
                offset,
                sequence.sampleId.data(),
                idLength))
        {
            sequences.clear();
            return false;
        }

        sequence.featureWidth =
            CompositionMidiTrainingEvent::featureCount;

        sequence.events.resize(
            eventCount);

        for (auto& event :
             sequence.events)
        {
            event.features.resize(
                CompositionMidiTrainingEvent::featureCount);

            if (!readBytes(
                    artifact.bytes,
                    offset,
                    event.features.data(),
                    event.features.size() *
                        sizeof(double)) ||
                !event.isValid())
            {
                sequences.clear();
                return false;
            }

            decodedEventCount++;
        }

        sequence.analysisValid =
            true;

        if (!sequence.isValid())
        {
            sequences.clear();
            return false;
        }

        sequences.push_back(
            std::move(sequence));
    }

    if (decodedEventCount != totalEventCount ||
        offset != artifact.bytes.size())
    {
        sequences.clear();
        return false;
    }

    return true;
}

} // namespace midigengx::music
