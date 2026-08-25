#include "CompositionNeuralModelArtifact.h"

#include <cstring>
#include <cmath>
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
    if (offset + sizeof(value) >
        bytes.size())
    {
        return false;
    }

    std::memcpy(
        &value,
        bytes.data() + offset,
        sizeof(value));

    offset += sizeof(value);
    return true;
}


bool safeByteCount(
    std::size_t elementCount,
    std::size_t elementSize,
    std::size_t& result) noexcept
{
    if (elementSize != 0 &&
        elementCount >
            std::numeric_limits<std::size_t>::max() /
                elementSize)
    {
        return false;
    }

    result =
        elementCount *
        elementSize;

    return true;
}

bool appendVector(
    std::vector<std::uint8_t>& bytes,
    std::size_t& offset,
    const std::vector<double>& values) noexcept
{
    std::size_t payloadSize = 0;

    if (!safeByteCount(
            values.size(),
            sizeof(double),
            payloadSize))
    {
        return false;
    }

    if (offset + payloadSize >
        bytes.size())
    {
        return false;
    }

    if (payloadSize != 0)
    {
        std::memcpy(
            bytes.data() + offset,
            values.data(),
            payloadSize);
    }

    offset += payloadSize;
    return true;
}

bool readVector(
    const std::vector<std::uint8_t>& bytes,
    std::size_t& offset,
    std::size_t elementCount,
    std::vector<double>& values) noexcept
{
    std::size_t payloadSize = 0;

    if (!safeByteCount(
            elementCount,
            sizeof(double),
            payloadSize))
    {
        return false;
    }

    if (offset + payloadSize >
        bytes.size())
    {
        return false;
    }

    values.resize(
        elementCount);

    if (payloadSize != 0)
    {
        std::memcpy(
            values.data(),
            bytes.data() + offset,
            payloadSize);
    }

    offset += payloadSize;
    return true;
}

std::size_t modelScalarCount(
    const CompositionNeuralModel& model) noexcept
{
    return model.inputWeights.size() +
           model.hiddenBias.size() +
           model.outputWeights.size() +
           model.outputBias.size() +
           model.contextResidualWeights.size();
}

bool isFiniteVector(
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

} // namespace

bool CompositionNeuralModelArtifact::isValid() const noexcept
{
    if (bytes.size() < headerSize)
        return false;

    std::size_t offset = 0;

    std::uint32_t artifactMagic = 0;
    std::uint32_t artifactVersion = 0;
    std::uint32_t objective = 0;
    std::uint32_t globalWidth = 0;
    std::uint32_t sectionWidth = 0;
    std::uint32_t targetWidth = 0;
    std::uint32_t hiddenWidth = 0;
    std::uint32_t scalarCount = 0;

    if (!readU32(bytes, offset, artifactMagic) ||
        !readU32(bytes, offset, artifactVersion) ||
        !readU32(bytes, offset, objective) ||
        !readU32(bytes, offset, globalWidth) ||
        !readU32(bytes, offset, sectionWidth) ||
        !readU32(bytes, offset, targetWidth) ||
        !readU32(bytes, offset, hiddenWidth) ||
        !readU32(bytes, offset, scalarCount))
    {
        return false;
    }

    if (artifactMagic !=
            CompositionNeuralModelArtifact::magic ||
        artifactVersion !=
            CompositionNeuralModelArtifact::version ||
        hiddenWidth == 0 ||
        scalarCount == 0)
    {
        return false;
    }

    std::size_t payloadSize = 0;

    if (!safeByteCount(
            scalarCount,
            sizeof(double),
            payloadSize))
    {
        return false;
    }

    return offset + payloadSize ==
           bytes.size() &&
           objective <=
               static_cast<std::uint32_t>(
                   LearningObjective::NextSectionPrediction);
}

CompositionNeuralModelArtifact
serializeCompositionNeuralModel(
    const CompositionNeuralModel& model) noexcept
{
    CompositionNeuralModelArtifact artifact;

    if (!model.isValid() ||
        !isFiniteVector(
            model.inputWeights) ||
        !isFiniteVector(
            model.hiddenBias) ||
        !isFiniteVector(
            model.outputWeights) ||
        !isFiniteVector(
            model.outputBias) ||
        !isFiniteVector(
            model.contextResidualWeights))
    {
        return artifact;
    }

    const auto scalarCount =
        modelScalarCount(model);

    std::size_t payloadSize = 0;

    if (!safeByteCount(
            scalarCount,
            sizeof(double),
            payloadSize))
    {
        return artifact;
    }

    if (payloadSize >
        std::numeric_limits<std::size_t>::max() -
            headerSize)
    {
        return artifact;
    }

    artifact.bytes.resize(
        headerSize +
        payloadSize);

    std::size_t offset = 0;

    writeU32(
        artifact.bytes,
        offset,
        CompositionNeuralModelArtifact::magic);

    writeU32(
        artifact.bytes,
        offset,
        CompositionNeuralModelArtifact::version);

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            model.contract.objective));

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            model.contract.globalInputWidth));

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            model.contract.sectionInputWidth));

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            model.contract.targetWidth));

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            CompositionNeuralModel::hiddenWidth));

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            scalarCount));

    if (!appendVector(
            artifact.bytes,
            offset,
            model.inputWeights) ||
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
            model.outputBias) ||
        !appendVector(
            artifact.bytes,
            offset,
            model.contextResidualWeights))
    {
        artifact.bytes.clear();
        return artifact;
    }

    return artifact;
}

bool deserializeCompositionNeuralModel(
    const CompositionNeuralModelArtifact& artifact,
    CompositionNeuralModel& model) noexcept
{
    if (!artifact.isValid())
        return false;

    std::size_t offset = 0;

    std::uint32_t artifactMagic = 0;
    std::uint32_t artifactVersion = 0;
    std::uint32_t objective = 0;
    std::uint32_t globalWidth = 0;
    std::uint32_t sectionWidth = 0;
    std::uint32_t targetWidth = 0;
    std::uint32_t hiddenWidth = 0;
    std::uint32_t scalarCount = 0;

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
            objective) ||
        !readU32(
            artifact.bytes,
            offset,
            globalWidth) ||
        !readU32(
            artifact.bytes,
            offset,
            sectionWidth) ||
        !readU32(
            artifact.bytes,
            offset,
            targetWidth) ||
        !readU32(
            artifact.bytes,
            offset,
            hiddenWidth) ||
        !readU32(
            artifact.bytes,
            offset,
            scalarCount))
    {
        return false;
    }

    if (artifactMagic !=
            CompositionNeuralModelArtifact::magic ||
        artifactVersion !=
            CompositionNeuralModelArtifact::version ||
        hiddenWidth !=
            CompositionNeuralModel::hiddenWidth)
    {
        return false;
    }

    CompositionLearningContract contract;

    contract.objective =
        static_cast<LearningObjective>(
            objective);

    contract.globalInputWidth =
        globalWidth;

    contract.sectionInputWidth =
        sectionWidth;

    contract.targetWidth =
        targetWidth;

    contract.analysisValid = true;
    contract.contextLength = 1;
    contract.usesSectionMask = true;

    if (!contract.isValid())
        return false;

    CompositionNeuralModel candidate;

    candidate.contract =
        contract;

    const auto inputWeightCount =
        (contract.globalInputWidth +
         contract.sectionInputWidth) *
        CompositionNeuralModel::hiddenWidth;

    const auto outputWeightCount =
        CompositionNeuralModel::hiddenWidth *
        contract.targetWidth;

    const auto residualWeightCount =
        contract.sectionInputWidth *
        contract.targetWidth;

    if (modelScalarCount(candidate) != 0)
        return false;

    if (scalarCount !=
            inputWeightCount +
            CompositionNeuralModel::hiddenWidth +
            outputWeightCount +
            contract.targetWidth +
            residualWeightCount)
    {
        return false;
    }

    if (!readVector(
            artifact.bytes,
            offset,
            inputWeightCount,
            candidate.inputWeights) ||
        !readVector(
            artifact.bytes,
            offset,
            CompositionNeuralModel::hiddenWidth,
            candidate.hiddenBias) ||
        !readVector(
            artifact.bytes,
            offset,
            outputWeightCount,
            candidate.outputWeights) ||
        !readVector(
            artifact.bytes,
            offset,
            contract.targetWidth,
            candidate.outputBias) ||
        !readVector(
            artifact.bytes,
            offset,
            residualWeightCount,
            candidate.contextResidualWeights))
    {
        return false;
    }

    if (offset != artifact.bytes.size() ||
        !isFiniteVector(
            candidate.inputWeights) ||
        !isFiniteVector(
            candidate.hiddenBias) ||
        !isFiniteVector(
            candidate.outputWeights) ||
        !isFiniteVector(
            candidate.outputBias) ||
        !isFiniteVector(
            candidate.contextResidualWeights))
    {
        return false;
    }

    candidate.initialized = true;

    if (!candidate.isValid())
        return false;

    model =
        std::move(candidate);

    return true;
}

} // namespace midigengx::music
