#include "CompositionNeuralModelRuntimeLoader.h"

namespace midigengx::music
{

bool CompositionNeuralModelRuntimeLoader::isValid() const noexcept
{
    return loaded &&
           model.isValid();
}

bool CompositionNeuralModelRuntimeLoader::load(
    const CompositionNeuralModelArtifact& artifact) noexcept
{
    CompositionNeuralModel candidate;

    if (!deserializeCompositionNeuralModel(
            artifact,
            candidate))
    {
        return false;
    }

    if (!candidate.isValid())
        return false;

    model =
        std::move(candidate);

    loaded =
        true;

    return true;
}

void CompositionNeuralModelRuntimeLoader::clear() noexcept
{
    model =
        CompositionNeuralModel{};

    loaded =
        false;
}

} // namespace midigengx::music
