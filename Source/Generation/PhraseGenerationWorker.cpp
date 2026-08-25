#include "PhraseGenerationWorker.h"

namespace midigengx::generation
{

PhraseGenerationWorker::PhraseGenerationWorker(
    ContextProvider provider,
    ResultHandler handler,
    GenerationProvider generation)
    : juce::Thread(
          "MIDI-GenGX Phrase Generator"),
      contextProvider(
          std::move(provider)),
      resultHandler(
          std::move(handler)),
      generationProvider(
          std::move(generation))
{
    startThread();
}

PhraseGenerationWorker::~PhraseGenerationWorker()
{
    signalThreadShouldExit();
    wakeUp.signal();
    stopThread(5000);
}

void PhraseGenerationWorker::request(
    std::uint64_t requestId,
    std::uint32_t seed) noexcept
{
    requestedSeed.store(
        seed,
        std::memory_order_release);

    requestedId.store(
        requestId,
        std::memory_order_release);

    wakeUp.signal();
}

void PhraseGenerationWorker::invalidate() noexcept
{
    requestedId.fetch_add(
        1,
        std::memory_order_acq_rel);

    wakeUp.signal();
}

void PhraseGenerationWorker::setGenerationProvider(
    GenerationProvider provider)
{
    generationProvider =
        std::move(provider);
}

void PhraseGenerationWorker::run()
{
    while (!threadShouldExit())
    {
        wakeUp.wait(-1);

        if (threadShouldExit())
            break;

        const auto requestId =
            requestedId.load(
                std::memory_order_acquire);

        const auto seed =
            requestedSeed.load(
                std::memory_order_acquire);

        if (requestId == 0)
            continue;

        const auto context =
            contextProvider();

        midigengx::music::Phrase phrase;

        if (generationProvider)
        {
            phrase =
                generationProvider(
                    context,
                    seed);
        }
        else
        {
            midigengx::music::MusicalEngine engine;

            phrase =
                engine.generate(
                    context,
                    seed);
        }

        if (requestId !=
            requestedId.load(
                std::memory_order_acquire))
        {
            continue;
        }

        if (threadShouldExit())
            break;

        resultHandler(
            std::move(phrase),
            requestId);
    }
}

} // namespace midigengx::generation
