#include "CompositionConditionedSequenceNeuralModelRuntimeLoader.h"

#include <cstring>

namespace midigengx::music
{

bool CompositionConditionedSequenceNeuralModelRuntimeLoadResult::isValid()
    const noexcept
{
    return valid &&
           artifact.isValid() &&
           model.isValid();
}

CompositionConditionedSequenceNeuralModelRuntimeLoadResult
CompositionConditionedSequenceNeuralModelRuntimeLoader::load(
    const std::vector<std::uint8_t>& embeddedBytes) const noexcept
{
    if (embeddedBytes.empty())
        return {};

    return load(
        embeddedBytes.data(),
        embeddedBytes.size());
}

CompositionConditionedSequenceNeuralModelRuntimeLoadResult
CompositionConditionedSequenceNeuralModelRuntimeLoader::load(
    const std::uint8_t* data,
    const std::size_t size) const noexcept
{
    CompositionConditionedSequenceNeuralModelRuntimeLoadResult result;

    if (data == nullptr ||
        size == 0 ||
        size > maxArtifactBytes)
    {
        return result;
    }

    result.artifact.bytes.resize(
        size);

    std::memcpy(
        result.artifact.bytes.data(),
        data,
        size);

    if (!result.artifact.isValid())
    {
        result.artifact.bytes.clear();
        return result;
    }

    if (!deserializeCompositionConditionedSequenceNeuralModel(
            result.artifact,
            result.model))
    {
        result.artifact.bytes.clear();
        result.model =
            CompositionConditionedSequenceNeuralModel{};
        return result;
    }

    result.valid =
        true;

    if (!result.isValid())
        result.valid = false;

    return result;
}

} // namespace
