#pragma once

#include "../Domain/MusicalContext.h"
#include "../Music/CompositionAIModelRuntimeProvider.h"
#include "../Music/CompositionNeuralModelArtifact.h"
#include "../Music/MusicalEngine.h"

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>

namespace midigengx::plugin
{

class AIRuntimeGeneration
{
public:
    using GenerationProvider =
        std::function<
            midigengx::music::Phrase(
                const midigengx::domain::MusicalContext&,
                std::uint32_t)>;

    void setEnabled(
        bool enabled) noexcept;

    bool isEnabled() const noexcept;

    void setProvider(
        GenerationProvider provider);

    bool hasProvider() const noexcept;

    bool loadModelArtifact(
        const midigengx::music::CompositionNeuralModelArtifact& artifact);

    bool hasLoadedModel() const noexcept;

    void clearModel();

    midigengx::music::Phrase generate(
        const midigengx::domain::MusicalContext& context,
        std::uint32_t seed) const;

private:
    std::atomic<bool> enabled{false};

    mutable std::mutex providerMutex;
    GenerationProvider provider;

    mutable std::mutex modelMutex;
    std::shared_ptr<
        const midigengx::music::CompositionAIModelRuntimeProvider>
        modelProvider;
};

} // namespace midigengx::plugin
