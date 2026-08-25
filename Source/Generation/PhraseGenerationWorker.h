#pragma once

#include <JuceHeader.h>

#include "../Domain/MusicalContext.h"
#include "../Music/MusicalEngine.h"

#include <atomic>
#include <cstdint>
#include <functional>

namespace midigengx::generation
{

class PhraseGenerationWorker final
    : private juce::Thread
{
public:
    using ContextProvider =
        std::function<
            midigengx::domain::MusicalContext()>;

    using ResultHandler =
        std::function<
            void(
                midigengx::music::Phrase&&,
                std::uint64_t)>;

    using GenerationProvider =
        std::function<
            midigengx::music::Phrase(
                const midigengx::domain::MusicalContext&,
                std::uint32_t)>;

    PhraseGenerationWorker(
        ContextProvider,
        ResultHandler,
        GenerationProvider = {});

    ~PhraseGenerationWorker() override;

    void request(
        std::uint64_t requestId,
        std::uint32_t seed) noexcept;

    void invalidate() noexcept;

    void setGenerationProvider(
        GenerationProvider provider);

private:
    void run() override;

    ContextProvider contextProvider;
    ResultHandler resultHandler;
    GenerationProvider generationProvider;

    std::atomic<std::uint64_t> requestedId{0};
    std::atomic<std::uint32_t> requestedSeed{0};

    juce::WaitableEvent wakeUp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        PhraseGenerationWorker)
};

} // namespace midigengx::generation
