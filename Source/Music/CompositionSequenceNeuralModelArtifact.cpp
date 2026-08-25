#include "CompositionSequenceNeuralModelArtifact.h"

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
        offset + byteCount > bytes.size())
    {
        return false;
    }

    if (byteCount != 0)
        std::memcpy(
            bytes.data() + offset,
            values.data(),
            byteCount);

    offset += byteCount;
    return true;
}

bool readVector(
    const std::vector<std::uint8_t>& bytes,
    std::size_t& offset,
    std::size_t count,
    std::vector<double>& values) noexcept
{
    std::size_t byteCount = 0;

    if (!safeMultiply(
            count,
            sizeof(double),
            byteCount) ||
        offset + byteCount > bytes.size())
    {
        return false;
    }

    values.resize(count);

    if (byteCount != 0)
        std::memcpy(
            values.data(),
            bytes.data() + offset,
            byteCount);

    offset += byteCount;
    return true;
}

} // namespace

bool CompositionSequenceNeuralModelArtifact::isValid()
    const noexcept
{
    if (bytes.size() < headerSize)
        return false;

    std::size_t offset = 0;

    std::uint32_t artifactMagic = 0;
    std::uint32_t artifactVersion = 0;
    std::uint32_t modelVersion = 0;
    std::uint32_t inputWidth = 0;
    std::uint32_t targetWidth = 0;
    std::uint32_t contextLength = 0;
    std::uint32_t hiddenWidth = 0;
    std::uint32_t parameterCount = 0;

    if (!readU32(bytes, offset, artifactMagic) ||
        !readU32(bytes, offset, artifactVersion) ||
        !readU32(bytes, offset, modelVersion) ||
        !readU32(bytes, offset, inputWidth) ||
        !readU32(bytes, offset, targetWidth) ||
        !readU32(bytes, offset, contextLength) ||
        !readU32(bytes, offset, hiddenWidth) ||
        !readU32(bytes, offset, parameterCount))
    {
        return false;
    }

    if (artifactMagic !=
            CompositionSequenceNeuralModelArtifact::magic ||
        artifactVersion !=
            CompositionSequenceNeuralModelArtifact::version ||
        modelVersion !=
            CompositionSequenceNeuralModel::version ||
        inputWidth !=
            CompositionMidiTrainingEvent::featureCount ||
        targetWidth !=
            CompositionMidiTrainingEvent::featureCount ||
        hiddenWidth !=
            CompositionSequenceNeuralModel::hiddenWidth ||
        contextLength < 4 ||
        contextLength > 4096 ||
        parameterCount == 0)
    {
        return false;
    }

    std::size_t expectedParameterCount = 0;
    std::size_t recurrentCount = 0;
    std::size_t outputCount = 0;

    if (!safeMultiply(
            static_cast<std::size_t>(hiddenWidth),
            static_cast<std::size_t>(inputWidth),
            expectedParameterCount) ||
        !safeMultiply(
            static_cast<std::size_t>(hiddenWidth),
            static_cast<std::size_t>(hiddenWidth),
            recurrentCount) ||
        !safeMultiply(
            static_cast<std::size_t>(targetWidth),
            static_cast<std::size_t>(hiddenWidth),
            outputCount))
    {
        return false;
    }

    expectedParameterCount +=
        static_cast<std::size_t>(hiddenWidth) +
        recurrentCount +
        outputCount +
        static_cast<std::size_t>(targetWidth);

    if (parameterCount !=
        expectedParameterCount)
    {
        return false;
    }

    std::size_t parameterBytes = 0;

    if (!safeMultiply(
            expectedParameterCount,
            sizeof(double),
            parameterBytes))
    {
        return false;
    }

    return offset + parameterBytes ==
           bytes.size();
}

CompositionSequenceNeuralModelArtifact
serializeCompositionSequenceNeuralModel(
    const CompositionSequenceNeuralModel& model)
    noexcept
{
    CompositionSequenceNeuralModelArtifact artifact;

    if (!model.isValid())
        return artifact;

    const auto inputCount =
        model.inputWeights.size();

    const auto recurrentCount =
        model.recurrentWeights.size();

    const auto hiddenBiasCount =
        model.hiddenBias.size();

    const auto outputCount =
        model.outputWeights.size();

    const auto outputBiasCount =
        model.outputBias.size();

    const auto parameterCount =
        inputCount +
        recurrentCount +
        hiddenBiasCount +
        outputCount +
        outputBiasCount;

    if (parameterCount >
        std::numeric_limits<std::uint32_t>::max())
    {
        return artifact;
    }

    std::size_t parameterBytes = 0;

    if (!safeMultiply(
            parameterCount,
            sizeof(double),
            parameterBytes) ||
        parameterBytes >
            std::numeric_limits<std::size_t>::max() -
                headerSize)
    {
        return artifact;
    }

    artifact.bytes.resize(
        headerSize +
        parameterBytes);

    std::size_t offset = 0;

    writeU32(
        artifact.bytes,
        offset,
        CompositionSequenceNeuralModelArtifact::magic);

    writeU32(
        artifact.bytes,
        offset,
        CompositionSequenceNeuralModelArtifact::version);

    writeU32(
        artifact.bytes,
        offset,
        CompositionSequenceNeuralModel::version);

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            model.contract.inputFeatureWidth));

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            model.contract.targetFeatureWidth));

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            model.contract.contextLength));

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            CompositionSequenceNeuralModel::hiddenWidth));

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            parameterCount));

    if (!appendVector(
            artifact.bytes,
            offset,
            model.inputWeights) ||
        !appendVector(
            artifact.bytes,
            offset,
            model.recurrentWeights) ||
        !appendVector(
            artifact.bytes,
            offset,
            model.hiddenBias) ||
        !appendVector(
            artifact.bytes,
            offset,
            model.outputWeights) ||
        !appendVector(
            artifact.bytes,
            offset,
            model.outputBias))
    {
        artifact.bytes.clear();
    }

    return artifact;
}

bool deserializeCompositionSequenceNeuralModel(
    const CompositionSequenceNeuralModelArtifact& artifact,
    CompositionSequenceNeuralModel& model)
    noexcept
{
    if (!artifact.isValid())
        return false;

    std::size_t offset = 0;

    std::uint32_t artifactMagic = 0;
    std::uint32_t artifactVersion = 0;
    std::uint32_t modelVersion = 0;
    std::uint32_t inputWidth = 0;
    std::uint32_t targetWidth = 0;
    std::uint32_t contextLength = 0;
    std::uint32_t hiddenWidth = 0;
    std::uint32_t parameterCount = 0;

    if (!readU32(artifact.bytes, offset, artifactMagic) ||
        !readU32(artifact.bytes, offset, artifactVersion) ||
        !readU32(artifact.bytes, offset, modelVersion) ||
        !readU32(artifact.bytes, offset, inputWidth) ||
        !readU32(artifact.bytes, offset, targetWidth) ||
        !readU32(artifact.bytes, offset, contextLength) ||
        !readU32(artifact.bytes, offset, hiddenWidth) ||
        !readU32(artifact.bytes, offset, parameterCount))
    {
        return false;
    }

    if (artifactMagic !=
            CompositionSequenceNeuralModelArtifact::magic ||
        artifactVersion !=
            CompositionSequenceNeuralModelArtifact::version ||
        modelVersion !=
            CompositionSequenceNeuralModel::version ||
        hiddenWidth !=
            CompositionSequenceNeuralModel::hiddenWidth)
    {
        return false;
    }

    const auto contract =
        buildCompositionSequenceLearningContract(
            static_cast<std::size_t>(
                contextLength),
            CompositionSequenceLearningObjective::NextEventPrediction);

    if (!contract.isValid() ||
        contract.inputFeatureWidth !=
            inputWidth ||
        contract.targetFeatureWidth !=
            targetWidth)
    {
        return false;
    }

    CompositionSequenceNeuralModel candidate =
        initializeCompositionSequenceNeuralModel(
            contract);

    if (!candidate.isValid())
        return false;

    if (!readVector(
            artifact.bytes,
            offset,
            candidate.inputWeights.size(),
            candidate.inputWeights) ||
        !readVector(
            artifact.bytes,
            offset,
            candidate.recurrentWeights.size(),
            candidate.recurrentWeights) ||
        !readVector(
            artifact.bytes,
            offset,
            candidate.hiddenBias.size(),
            candidate.hiddenBias) ||
        !readVector(
            artifact.bytes,
            offset,
            candidate.outputWeights.size(),
            candidate.outputWeights) ||
        !readVector(
            artifact.bytes,
            offset,
            candidate.outputBias.size(),
            candidate.outputBias) ||
        offset != artifact.bytes.size())
    {
        return false;
    }

    if (!candidate.isValid())
        return false;

    model =
        std::move(candidate);

    return true;
}

} // namespace midigengx::music
