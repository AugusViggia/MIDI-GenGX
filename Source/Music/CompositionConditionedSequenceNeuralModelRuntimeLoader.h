#include "CompositionConditionedSequenceNeuralModelArtifact.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace midigengx::music
{

struct CompositionConditionedSequenceNeuralModelRuntimeLoadResult
{
    CompositionConditionedSequenceNeuralModel model;
    CompositionConditionedSequenceNeuralModelArtifact artifact;

    bool valid = false;

    bool isValid() const noexcept;
};

class CompositionConditionedSequenceNeuralModelRuntimeLoader
{
public:
    static constexpr int version = 1;
    static constexpr std::size_t maxArtifactBytes = 256u * 1024u * 1024u;

    CompositionConditionedSequenceNeuralModelRuntimeLoadResult
    load(
        const std::vector<std::uint8_t>& embeddedBytes) const noexcept;

    CompositionConditionedSequenceNeuralModelRuntimeLoadResult
    load(
        const std::uint8_t* data,
        std::size_t size) const noexcept;
};

} // namespace
