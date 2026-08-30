#include "CompositionAIModelRuntimeProvider.h"

namespace midigengx::music
{

bool CompositionAIModelRuntimeProvider::isReady() const noexcept
{
    return loader.isValid() &&
           pipeline.isValid() &&
           coordinator.isValid();
}

bool CompositionAIModelRuntimeProvider::load(
    const CompositionNeuralModelArtifact& artifact) noexcept
{
    CompositionNeuralModelRuntimeLoader candidateLoader;

    if (!candidateLoader.load(
            artifact))
    {
        return false;
    }

    const auto candidatePipeline =
        buildCompositionInferencePipeline(
            candidateLoader.model);

    if (!candidatePipeline.isValid())
        return false;

    const auto candidateBridge =
        buildCompositionAIEngineBridge(
            true);

    const auto candidateCoordinator =
        buildCompositionAIGenerationCoordinator(
            candidatePipeline,
            candidateBridge,
            true);

    if (!candidateCoordinator.isValid())
        return false;

    loader =
        std::move(candidateLoader);

    pipeline =
        candidatePipeline;

    coordinator =
        candidateCoordinator;

    return true;
}

void CompositionAIModelRuntimeProvider::clear() noexcept
{
    coordinator =
        CompositionAIGenerationCoordinator{};

    pipeline =
        CompositionInferencePipeline{};

    loader.clear();
}

Phrase CompositionAIModelRuntimeProvider::generate(
    const midigengx::domain::MusicalContext& context,
    std::uint32_t seed) const noexcept
{
    if (!isReady())
    {
        MusicalEngine engine;

        return engine.generate(
            context,
            seed);
    }

    // Convert the runtime context using the exact same runtime feature
    // contract used by the standalone inference service.
    const auto features =
        CompositionRuntimeFeatureAdapter{}
            .build(context);

    if (!features.isValid())
    {
        MusicalEngine engine;

        return engine.generate(
            context,
            seed);
    }

    const auto request =
        CompositionRuntimeInferenceAdapter{}
            .buildRequest(
                features,
                pipeline.contract);

    if (!request.isValid(
            pipeline.contract))
    {
        MusicalEngine engine;

        return engine.generate(
            context,
            seed);
    }

    const auto inference =
        pipeline.infer(
            request);

    if (!inference.isValid(
            pipeline.contract))
    {
        MusicalEngine engine;

        return engine.generate(
            context,
            seed);
    }

    CompositionAIGenerationRequest generationRequest;

    generationRequest.context =
        context;

    generationRequest.globalFeatures =
        request.globalFeatures;

    generationRequest.contextSectionFeatures =
        request.contextSectionFeatures;

    generationRequest.contextIsValid =
        request.contextIsValid;

    generationRequest.seed =
        seed;

    const auto result =
        coordinator.generate(
            generationRequest);

    if (result.isValid())
    {
        auto phrase = result.phrase;
        MusicalEngine::constrainPhraseToMusicalContext(
            phrase,
            context);
        return phrase;
    }

    MusicalEngine engine;

    return engine.generate(
        context,
        seed);
}

} // namespace midigengx::music
