#include "Generation/PhraseGenerationWorker.h"
#include "Plugin/AIRuntimeGeneration.h"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

using namespace midigengx;

namespace
{

void expect(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr
            << "FAILED: "
            << message
            << '\n';
        std::exit(1);
    }
}

domain::MusicalContext buildContext()
{
    domain::MusicalContext context;

    context.key = domain::Key::C;
    context.scale = domain::Scale{
        domain::ScaleType::Minor};
    context.role = domain::Role::Lead;

    context.parameters.lengthBars = 4;
    context.parameters.phraseLengthBars = 4;
    context.parameters.octaveLow = 0;
    context.parameters.octaveHigh = 1;
    context.parameters.density = 50;
    context.parameters.tension = 50;
    context.parameters.variation = 50;
    context.parameters.complexity = 50;

    context.normalize();
    return context;
}

music::Phrase buildMarkerPhrase(
    int midi)
{
    music::Phrase phrase;
    phrase.lengthBeats = 4.0;

    music::NoteEvent note;
    note.startBeat = 0.0;
    note.durationBeats = 1.0;
    note.midiNote = midi;
    note.velocity = 100;

    phrase.notes.push_back(note);
    phrase.normalize();

    return phrase;
}

void waitForCallback(
    const std::atomic<int>& callbackCount)
{
    for (int i = 0;
         i < 100 &&
         callbackCount.load(
             std::memory_order_acquire) == 0;
         ++i)
    {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(10));
    }
}

void testRuntimeFallsBackWhenDisabled()
{
    plugin::AIRuntimeGeneration runtime;
    std::atomic<bool> called{false};

    runtime.setProvider(
        [&called](
            const domain::MusicalContext&,
            std::uint32_t)
        {
            called.store(
                true,
                std::memory_order_release);

            return buildMarkerPhrase(72);
        });

    runtime.setEnabled(false);

    const auto phrase =
        runtime.generate(
            buildContext(),
            1);

    expect(
        !called.load(
            std::memory_order_acquire),
        "disabled runtime does not call AI provider");

    expect(
        phrase.isValid(),
        "disabled runtime returns valid baseline phrase");
}

void testRuntimeUsesProviderWhenEnabled()
{
    plugin::AIRuntimeGeneration runtime;
    std::atomic<bool> called{false};

    runtime.setProvider(
        [&called](
            const domain::MusicalContext&,
            std::uint32_t)
        {
            called.store(
                true,
                std::memory_order_release);

            return buildMarkerPhrase(84);
        });

    runtime.setEnabled(true);

    const auto phrase =
        runtime.generate(
            buildContext(),
            99);

    expect(
        called.load(
            std::memory_order_acquire),
        "enabled runtime calls AI provider");

    expect(
        phrase.isValid(),
        "AI provider result is returned");

    expect(
        !phrase.notes.empty() &&
        phrase.notes.front().midiNote == 84,
        "AI provider output reaches runtime");
}

void testRuntimeProviderCanBeReplaced()
{
    plugin::AIRuntimeGeneration runtime;

    runtime.setProvider(
        [](
            const domain::MusicalContext&,
            std::uint32_t)
        {
            return buildMarkerPhrase(60);
        });

    runtime.setEnabled(true);

    const auto first =
        runtime.generate(
            buildContext(),
            1);

    runtime.setProvider(
        [](
            const domain::MusicalContext&,
            std::uint32_t)
        {
            return buildMarkerPhrase(67);
        });

    const auto second =
        runtime.generate(
            buildContext(),
            1);

    expect(
        first.notes.front().midiNote == 60 &&
        second.notes.front().midiNote == 67,
        "runtime provider replacement is effective");
}

void testWorkerUsesInjectedGenerationProvider()
{
    std::atomic<int> callbackCount{0};
    std::atomic<int> receivedMidi{-1};
    std::atomic<std::uint64_t> receivedId{0};

    generation::PhraseGenerationWorker worker(
        []()
        {
            return buildContext();
        },
        [&](
            music::Phrase&& phrase,
            std::uint64_t id)
        {
            callbackCount.fetch_add(
                1,
                std::memory_order_acq_rel);

            if (!phrase.notes.empty())
            {
                receivedMidi.store(
                    phrase.notes.front().midiNote,
                    std::memory_order_release);
            }

            receivedId.store(
                id,
                std::memory_order_release);
        },
        [](
            const domain::MusicalContext&,
            std::uint32_t)
        {
            return buildMarkerPhrase(91);
        });

    worker.request(
        17,
        123);

    waitForCallback(
        callbackCount);

    expect(
        callbackCount.load(
            std::memory_order_acquire) == 1,
        "generation worker invokes injected provider");

    expect(
        receivedMidi.load(
            std::memory_order_acquire) == 91,
        "worker publishes injected provider output");

    expect(
        receivedId.load(
            std::memory_order_acquire) == 17,
        "worker preserves request id");
}

void testWorkerFallbackWithoutProvider()
{
    std::atomic<int> callbackCount{0};

    generation::PhraseGenerationWorker worker(
        []()
        {
            return buildContext();
        },
        [&](
            music::Phrase&& phrase,
            std::uint64_t)
        {
            if (phrase.isValid())
            {
                callbackCount.fetch_add(
                    1,
                    std::memory_order_acq_rel);
            }
        });

    worker.request(
        22,
        456);

    waitForCallback(
        callbackCount);

    expect(
        callbackCount.load(
            std::memory_order_acquire) == 1,
        "worker retains baseline fallback without provider");
}

} // namespace

int main()
{
    testRuntimeFallsBackWhenDisabled();
    testRuntimeUsesProviderWhenEnabled();
    testRuntimeProviderCanBeReplaced();
    testWorkerUsesInjectedGenerationProvider();
    testWorkerFallbackWithoutProvider();

    std::cout
        << "MIDI-GenGX AI plugin runtime integration tests passed.\n";

    return 0;
}
