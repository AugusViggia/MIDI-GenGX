#include "AIRuntimeGeneration.h"

namespace midigengx::plugin
{

void AIRuntimeGeneration::setEnabled(
    bool value) noexcept
{
    enabled.store(
        value,
        std::memory_order_release);
}

bool AIRuntimeGeneration::isEnabled() const noexcept
{
    return enabled.load(
        std::memory_order_acquire);
}

void AIRuntimeGeneration::setProvider(
    GenerationProvider nextProvider)
{
    std::scoped_lock lock(
        providerMutex);

    provider =
        std::move(nextProvider);
}

bool AIRuntimeGeneration::hasProvider() const noexcept
{
    {
        std::scoped_lock lock(
            providerMutex);

        if (provider)
            return true;
    }

    std::scoped_lock lock(
        modelMutex);

    return static_cast<bool>(
        modelProvider);
}

bool AIRuntimeGeneration::loadModelArtifact(
    const midigengx::music::CompositionNeuralModelArtifact& artifact)
{
    auto candidate =
        std::make_shared<
            midigengx::music::CompositionAIModelRuntimeProvider>();

    if (!candidate->load(
            artifact))
    {
        return false;
    }

    std::scoped_lock lock(
        modelMutex);

    modelProvider =
        std::move(candidate);

    return true;
}

bool AIRuntimeGeneration::hasLoadedModel() const noexcept
{
    std::scoped_lock lock(
        modelMutex);

    return static_cast<bool>(
               modelProvider) &&
           modelProvider->isReady();
}

void AIRuntimeGeneration::clearModel()
{
    std::scoped_lock lock(
        modelMutex);

    modelProvider.reset();
}

midigengx::music::Phrase
AIRuntimeGeneration::generate(
    const midigengx::domain::MusicalContext& context,
    std::uint32_t seed) const
{
    if (!isEnabled())
    {
        midigengx::music::MusicalEngine engine;

        return engine.generate(
            context,
            seed);
    }

    GenerationProvider currentProvider;

    {
        std::scoped_lock lock(
            providerMutex);

        currentProvider =
            provider;
    }

    if (currentProvider)
    {
        return currentProvider(
            context,
            seed);
    }

    std::shared_ptr<
        const midigengx::music::CompositionAIModelRuntimeProvider>
        currentModel;

    {
        std::scoped_lock lock(
            modelMutex);

        currentModel =
            modelProvider;
    }

    if (currentModel &&
        currentModel->isReady())
    {
        return currentModel->generate(
            context,
            seed);
    }

    midigengx::music::MusicalEngine engine;

    return engine.generate(
        context,
        seed);
}

} // namespace midigengx::plugin
