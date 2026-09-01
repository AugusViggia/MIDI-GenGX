#include "CompositionConditionedSequenceNeuralModelArtifact.h"

#include <array>
#include <cstring>
#include <limits>

namespace midigengx::music
{
namespace
{

constexpr std::size_t headerSize =
    sizeof(std::uint32_t) * 15;

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
    std::size_t left,
    std::size_t right,
    std::size_t& result) noexcept
{
    if (right != 0 &&
        left >
            std::numeric_limits<std::size_t>::max() /
            right)
    {
        return false;
    }

    result =
        left *
        right;

    return true;
}

bool appendString(
    std::vector<std::uint8_t>& bytes,
    std::size_t& offset,
    const std::string& value) noexcept
{
    if (value.size() >
        std::numeric_limits<std::uint32_t>::max())
    {
        return false;
    }

    const auto length =
        static_cast<std::uint32_t>(
            value.size());

    if (offset + sizeof(length) +
            value.size() >
        bytes.size())
    {
        return false;
    }

    std::memcpy(
        bytes.data() + offset,
        &length,
        sizeof(length));

    offset +=
        sizeof(length);

    if (!value.empty())
    {
        std::memcpy(
            bytes.data() + offset,
            value.data(),
            value.size());

        offset +=
            value.size();
    }

    return true;
}

bool readString(
    const std::vector<std::uint8_t>& bytes,
    std::size_t& offset,
    std::string& value) noexcept
{
    std::uint32_t length = 0;

    if (!readU32(
            bytes,
            offset,
            length) ||
        offset + length >
            bytes.size())
    {
        return false;
    }

    value.assign(
        reinterpret_cast<const char*>(
            bytes.data() + offset),
        length);

    offset +=
        length;

    return true;
}

std::size_t stringTableBytes(
    const std::vector<std::string>& values) noexcept
{
    std::size_t result = 0;

    for (const auto& value :
         values)
    {
        if (value.size() >
            std::numeric_limits<std::uint32_t>::max() ||
            result >
                std::numeric_limits<std::size_t>::max() -
                sizeof(std::uint32_t) -
                value.size())
        {
            return 0;
        }

        result +=
            sizeof(std::uint32_t) +
            value.size();
    }

    return result;
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

    offset +=
        byteCount;

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
        offset + byteCount >
            bytes.size())
    {
        return false;
    }

    values.resize(
        count);

    if (byteCount != 0)
    {
        std::memcpy(
            values.data(),
            bytes.data() + offset,
            byteCount);
    }

    offset +=
        byteCount;

    return true;
}

} // namespace

bool CompositionConditionedSequenceNeuralModelArtifact::isValid()
    const noexcept
{
    if (bytes.size() <
        headerSize)
    {
        return false;
    }

    std::size_t offset = 0;

    std::uint32_t artifactMagic = 0;
    std::uint32_t artifactVersion = 0;
    std::uint32_t modelVersion = 0;
    std::uint32_t contractVersion = 0;
    std::uint32_t contextLength = 0;
    std::uint32_t inputWidth = 0;
    std::uint32_t targetWidth = 0;
    std::uint32_t hiddenWidth = 0;
    std::uint32_t embeddingWidth = 0;
    std::uint32_t knowledgeFeatureCount = 0;
    std::uint32_t knowledgeEmbeddingWidth = 0;
    std::uint32_t composerCount = 0;
    std::uint32_t styleCount = 0;
    std::uint32_t eraCount = 0;

    if (!readU32(bytes, offset, artifactMagic) ||
        !readU32(bytes, offset, artifactVersion) ||
        !readU32(bytes, offset, modelVersion) ||
        !readU32(bytes, offset, contractVersion) ||
        !readU32(bytes, offset, contextLength) ||
        !readU32(bytes, offset, inputWidth) ||
        !readU32(bytes, offset, targetWidth) ||
        !readU32(bytes, offset, hiddenWidth) ||
        !readU32(bytes, offset, embeddingWidth) ||
        !readU32(bytes, offset, knowledgeFeatureCount) ||
        !readU32(bytes, offset, knowledgeEmbeddingWidth) ||
        !readU32(bytes, offset, composerCount) ||
        !readU32(bytes, offset, styleCount) ||
        !readU32(bytes, offset, eraCount))
    {
        return false;
    }

    if (artifactMagic !=
            CompositionConditionedSequenceNeuralModelArtifact::magic ||
        artifactVersion !=
            CompositionConditionedSequenceNeuralModelArtifact::version ||
        modelVersion !=
            CompositionConditionedSequenceNeuralModel::version ||
        contractVersion !=
            CompositionSequenceLearningContract::version ||
        contextLength < 4 ||
        contextLength > 4096 ||
        inputWidth !=
            CompositionMidiTrainingEvent::featureCount ||
        targetWidth !=
            CompositionMidiTrainingEvent::featureCount ||
        hiddenWidth !=
            CompositionConditionedSequenceNeuralModel::hiddenWidth ||
        embeddingWidth !=
            CompositionConditionedSequenceNeuralModel::conditionEmbeddingWidth ||
        knowledgeFeatureCount !=
            CompositionConditionedSequenceNeuralModel::knowledgeFeatureCount ||
        knowledgeEmbeddingWidth !=
            CompositionConditionedSequenceNeuralModel::knowledgeEmbeddingWidth ||
        composerCount == 0 ||
        styleCount == 0 ||
        eraCount == 0)
    {
        return false;
    }

    if (offset + sizeof(std::uint32_t) >
        bytes.size())
    {
        return false;
    }

    std::uint32_t instrumentationCount = 0;

    if (!readU32(
            bytes,
            offset,
            instrumentationCount) ||
        instrumentationCount == 0)
    {
        return false;
    }

    const std::size_t featureWidth =
        static_cast<std::size_t>(
            inputWidth);

    const std::size_t hidden =
        static_cast<std::size_t>(
            hiddenWidth);

    std::size_t parameterCount = 0;
    std::size_t temporary = 0;

    if (!safeMultiply(
            hidden,
            featureWidth,
            temporary))
    {
        return false;
    }

    parameterCount +=
        temporary;

    if (!safeMultiply(
            hidden,
            hidden,
            temporary))
    {
        return false;
    }

    parameterCount +=
        temporary +
        hidden;

    if (!safeMultiply(
            static_cast<std::size_t>(
                targetWidth),
            hidden,
            temporary))
    {
        return false;
    }

    parameterCount +=
        temporary +
        static_cast<std::size_t>(
            targetWidth);

    if (!safeMultiply(
            static_cast<std::size_t>(
                composerCount),
            static_cast<std::size_t>(
                embeddingWidth),
            temporary))
    {
        return false;
    }

    parameterCount +=
        temporary;

    if (!safeMultiply(
            CompositionConditionedSequenceNeuralModel::knowledgeEmbeddingWidth,
            CompositionConditionedSequenceNeuralModel::knowledgeFeatureCount,
            temporary))
    {
        return false;
    }

    parameterCount +=
        temporary;

    if (!safeMultiply(
            static_cast<std::size_t>(
                styleCount),
            static_cast<std::size_t>(
                embeddingWidth),
            temporary))
    {
        return false;
    }

    parameterCount +=
        temporary;

    if (!safeMultiply(
            static_cast<std::size_t>(
                eraCount),
            static_cast<std::size_t>(
                embeddingWidth),
            temporary))
    {
        return false;
    }

    parameterCount +=
        temporary;

    if (!safeMultiply(
            static_cast<std::size_t>(
                instrumentationCount),
            static_cast<std::size_t>(
                embeddingWidth),
            temporary))
    {
        return false;
    }

    parameterCount +=
        temporary;

    std::size_t parameterBytes = 0;

    if (!safeMultiply(
            parameterCount,
            sizeof(double),
            parameterBytes))
    {
        return false;
    }

    const auto stringCount =
        static_cast<std::size_t>(
            composerCount) +
        static_cast<std::size_t>(
            styleCount) +
        static_cast<std::size_t>(
            eraCount) +
        static_cast<std::size_t>(
            instrumentationCount);

    if (stringCount == 0)
        return false;

    // Validate the string table structurally without allocating unbounded data.
    std::size_t stringOffset =
        offset + parameterBytes;

    for (std::size_t index = 0;
         index < stringCount;
         ++index)
    {
        std::uint32_t length = 0;

        if (!readU32(
                bytes,
                stringOffset,
                length) ||
            stringOffset + length >
                bytes.size())
        {
            return false;
        }

        stringOffset +=
            length;

        if (length == 0)
            return false;
    }

    return stringOffset ==
           bytes.size();
}

CompositionConditionedSequenceNeuralModelArtifact
serializeCompositionConditionedSequenceNeuralModel(
    const CompositionConditionedSequenceNeuralModel& model)
    noexcept
{
    CompositionConditionedSequenceNeuralModelArtifact artifact;

    if (!model.isValid())
        return artifact;

    const std::size_t counts[4] =
    {
        model.vocabulary.composers.size(),
        model.vocabulary.styles.size(),
        model.vocabulary.eras.size(),
        model.vocabulary.instrumentations.size()
    };

    for (const auto count :
         counts)
    {
        if (count >
            std::numeric_limits<std::uint32_t>::max())
        {
            return artifact;
        }
    }

    const std::size_t vectors[] =
    {
        model.inputWeights.size(),
        model.recurrentWeights.size(),
        model.hiddenBias.size(),
        model.outputWeights.size(),
        model.outputBias.size(),
        model.composerEmbeddings.size(),
        model.styleEmbeddings.size(),
        model.eraEmbeddings.size(),
        model.instrumentationEmbeddings.size()
    };

    std::size_t parameterCount = 0;

    for (const auto count :
         vectors)
    {
        if (parameterCount >
            std::numeric_limits<std::size_t>::max() -
                count)
        {
            return artifact;
        }

        parameterCount +=
            count;
    }

    std::size_t parameterBytes = 0;

    if (!safeMultiply(
            parameterCount,
            sizeof(double),
            parameterBytes))
    {
        return artifact;
    }

    const std::array<std::vector<std::string>, 4>
        vocabularyStrings =
    {
        model.vocabulary.composers,
        model.vocabulary.styles,
        model.vocabulary.eras,
        model.vocabulary.instrumentations
    };

    std::size_t stringBytes = 0;

    for (const auto& category :
         vocabularyStrings)
    {
        const auto categoryBytes =
            stringTableBytes(category);

        if (categoryBytes == 0 ||
            stringBytes >
                std::numeric_limits<std::size_t>::max() -
                categoryBytes)
        {
            return artifact;
        }

        stringBytes +=
            categoryBytes;
    }

    if (parameterBytes >
            std::numeric_limits<std::size_t>::max() -
            headerSize ||
        stringBytes >
            std::numeric_limits<std::size_t>::max() -
            headerSize -
            parameterBytes)
    {
        return artifact;
    }

    artifact.bytes.resize(
        headerSize +
        parameterBytes +
        stringBytes);

    std::size_t offset = 0;

    writeU32(
        artifact.bytes,
        offset,
        CompositionConditionedSequenceNeuralModelArtifact::magic);

    writeU32(
        artifact.bytes,
        offset,
        CompositionConditionedSequenceNeuralModelArtifact::version);

    writeU32(
        artifact.bytes,
        offset,
        CompositionConditionedSequenceNeuralModel::version);

    writeU32(
        artifact.bytes,
        offset,
        CompositionSequenceLearningContract::version);

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            model.contract.contextLength));

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
            CompositionConditionedSequenceNeuralModel::hiddenWidth));

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            CompositionConditionedSequenceNeuralModel::conditionEmbeddingWidth));

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            counts[0]));

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            counts[1]));

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            counts[2]));

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            counts[3]));

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            CompositionConditionedSequenceNeuralModel::knowledgeFeatureCount));

    writeU32(
        artifact.bytes,
        offset,
        static_cast<std::uint32_t>(
            CompositionConditionedSequenceNeuralModel::knowledgeEmbeddingWidth));

    const std::vector<double>* parameters[] =
    {
        &model.inputWeights,
        &model.recurrentWeights,
        &model.hiddenBias,
        &model.outputWeights,
        &model.outputBias,
        &model.composerEmbeddings,
        &model.styleEmbeddings,
        &model.eraEmbeddings,
        &model.instrumentationEmbeddings,
        &model.knowledgeProjectionWeights
    };

    for (const auto* parameter :
         parameters)
    {
        if (!appendVector(
                artifact.bytes,
                offset,
                *parameter))
        {
            artifact.bytes.clear();
            return artifact;
        }
    }

    for (const auto& category :
         vocabularyStrings)
    {
        for (const auto& value :
             category)
        {
            if (!appendString(
                    artifact.bytes,
                    offset,
                    value))
            {
                artifact.bytes.clear();
                return artifact;
            }
        }
    }

    return artifact;
}

bool deserializeCompositionConditionedSequenceNeuralModel(
    const CompositionConditionedSequenceNeuralModelArtifact& artifact,
    CompositionConditionedSequenceNeuralModel& model)
    noexcept
{
    model =
        CompositionConditionedSequenceNeuralModel{};

    if (!artifact.isValid())
        return false;

    constexpr std::size_t headerSize =
        sizeof(std::uint32_t) * 15;

    std::size_t headerOffset = 0;

    std::uint32_t values[15] =
    {
        0
    };

    for (auto& value :
         values)
    {
        if (!readU32(
                artifact.bytes,
                headerOffset,
                value))
        {
            return false;
        }
    }

    const auto contract =
        buildCompositionSequenceLearningContract(
            values[4],
            CompositionSequenceLearningObjective::NextEventPrediction);

    if (!contract.isValid() ||
        values[5] !=
            CompositionMidiTrainingEvent::featureCount ||
        values[6] !=
            CompositionMidiTrainingEvent::featureCount ||
        values[7] !=
            CompositionConditionedSequenceNeuralModel::hiddenWidth ||
        values[8] !=
            CompositionConditionedSequenceNeuralModel::conditionEmbeddingWidth)
    {
        return false;
    }

    if (values[9] != CompositionConditionedSequenceNeuralModel::knowledgeFeatureCount ||
        values[10] != CompositionConditionedSequenceNeuralModel::knowledgeEmbeddingWidth)
    {
        return false;
    }

    const std::size_t composerCount =
        static_cast<std::size_t>(values[11]);

    const std::size_t styleCount =
        static_cast<std::size_t>(values[12]);

    const std::size_t eraCount =
        static_cast<std::size_t>(values[13]);

    const std::size_t instrumentationCount =
        static_cast<std::size_t>(values[14]);

    const std::size_t hidden =
        static_cast<std::size_t>(
            CompositionConditionedSequenceNeuralModel::hiddenWidth);

    const std::size_t inputWidth =
        static_cast<std::size_t>(
            CompositionMidiTrainingEvent::featureCount);

    const std::size_t targetWidth =
        inputWidth;

    const std::size_t embeddingWidth =
        static_cast<std::size_t>(
            CompositionConditionedSequenceNeuralModel::conditionEmbeddingWidth);

    std::size_t parameterCount = 0;
    std::size_t temporary = 0;

    if (!safeMultiply(
            hidden,
            inputWidth,
            temporary))
    {
        return false;
    }

    parameterCount +=
        temporary;

    if (!safeMultiply(
            hidden,
            hidden,
            temporary))
    {
        return false;
    }

    parameterCount +=
        temporary +
        hidden;

    if (!safeMultiply(
            targetWidth,
            hidden,
            temporary))
    {
        return false;
    }

    parameterCount +=
        temporary +
        targetWidth;

    const std::size_t categoryCounts[4] =
    {
        composerCount,
        styleCount,
        eraCount,
        instrumentationCount
    };

    for (const auto count :
         categoryCounts)
    {
        if (!safeMultiply(
                count,
                embeddingWidth,
                temporary))
        {
            return false;
        }

        parameterCount +=
            temporary;
    }

    std::size_t parameterBytes = 0;

    if (!safeMultiply(
            parameterCount,
            sizeof(double),
            parameterBytes))
    {
        return false;
    }

    if (headerSize >
            artifact.bytes.size() ||
        parameterBytes >
            artifact.bytes.size() -
            headerSize)
    {
        return false;
    }

    const auto stringOffset =
        headerSize +
        parameterBytes;

    // The serializer stores all learned parameters first and the actual
    // vocabulary strings afterward. Read the vocabulary from that second
    // region before constructing the candidate model.
    std::size_t vocabularyOffset =
        stringOffset;

    CompositionConditioningVocabulary vocabulary;

    vocabulary.composers.resize(
        composerCount);

    vocabulary.styles.resize(
        styleCount);

    vocabulary.eras.resize(
        eraCount);

    vocabulary.instrumentations.resize(
        instrumentationCount);

    for (std::size_t index = 0;
         index < composerCount;
         ++index)
    {
        if (!readString(
                artifact.bytes,
                vocabularyOffset,
                vocabulary.composers[index]))
        {
            return false;
        }
    }

    for (std::size_t index = 0;
         index < styleCount;
         ++index)
    {
        if (!readString(
                artifact.bytes,
                vocabularyOffset,
                vocabulary.styles[index]))
        {
            return false;
        }
    }

    for (std::size_t index = 0;
         index < eraCount;
         ++index)
    {
        if (!readString(
                artifact.bytes,
                vocabularyOffset,
                vocabulary.eras[index]))
        {
            return false;
        }
    }

    for (std::size_t index = 0;
         index < instrumentationCount;
         ++index)
    {
        if (!readString(
                artifact.bytes,
                vocabularyOffset,
                vocabulary.instrumentations[index]))
        {
            return false;
        }
    }

    if (vocabularyOffset !=
        artifact.bytes.size())
    {
        return false;
    }

    vocabulary.valid =
        true;

    CompositionConditionedSequenceNeuralModel candidate =
        initializeCompositionConditionedSequenceNeuralModel(
            contract,
            vocabulary);

    if (!candidate.isValid())
        return false;

    // Parameters occupy the region immediately after the numeric header.
    // Reset the read cursor to the parameter region; vocabulary was read from
    // the separate trailing string table above.
    std::size_t parameterOffset =
        headerSize;

    if (!readVector(
            artifact.bytes,
            parameterOffset,
            candidate.inputWeights.size(),
            candidate.inputWeights) ||
        !readVector(
            artifact.bytes,
            parameterOffset,
            candidate.recurrentWeights.size(),
            candidate.recurrentWeights) ||
        !readVector(
            artifact.bytes,
            parameterOffset,
            candidate.hiddenBias.size(),
            candidate.hiddenBias) ||
        !readVector(
            artifact.bytes,
            parameterOffset,
            candidate.outputWeights.size(),
            candidate.outputWeights) ||
        !readVector(
            artifact.bytes,
            parameterOffset,
            candidate.outputBias.size(),
            candidate.outputBias) ||
        !readVector(
            artifact.bytes,
            parameterOffset,
            candidate.composerEmbeddings.size(),
            candidate.composerEmbeddings) ||
        !readVector(
            artifact.bytes,
            parameterOffset,
            candidate.styleEmbeddings.size(),
            candidate.styleEmbeddings) ||
        !readVector(
            artifact.bytes,
            parameterOffset,
            candidate.eraEmbeddings.size(),
            candidate.eraEmbeddings) ||
        !readVector(
            artifact.bytes,
            parameterOffset,
            candidate.instrumentationEmbeddings.size(),
            candidate.instrumentationEmbeddings) ||
        !readVector(
            artifact.bytes,
            parameterOffset,
            candidate.knowledgeProjectionWeights.size(),
            candidate.knowledgeProjectionWeights) ||
        parameterOffset != stringOffset)
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
