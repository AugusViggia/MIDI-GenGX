#include "../Generation/GenerationActivationPolicy.h"

#include <cmath>
#include <iostream>

namespace
{
int failures = 0;

void expect(
    bool condition,
    const char* message)
{
    if (!condition)
    {
        ++failures;
        std::cerr
            << "FAILED: "
            << message
            << "\n";
    }
}

void testInitialPhraseCanAdoptDuringPlayback()
{
    expect(
        midigengx::generation::
            shouldAdoptPublishedPhrase(
                false,
                true,
                10.25,
                10.0,
                16.0),
        "first generated phrase can be adopted during playback");
}

void testReplacementWaitsForPhraseBoundary()
{
    expect(
        !midigengx::generation::
            shouldAdoptPublishedPhrase(
                true,
                true,
                10.25,
                10.0,
                16.0),
        "active phrase is not replaced mid-phrase");

    expect(
        midigengx::generation::
            shouldAdoptPublishedPhrase(
                true,
                true,
                16.10,
                15.90,
                16.0),
        "replacement is adopted after phrase boundary");
}

void testStoppedTransportCanAdoptImmediately()
{
    expect(
        midigengx::generation::
            shouldAdoptPublishedPhrase(
                true,
                false,
                7.0,
                6.5,
                16.0),
        "stopped transport can adopt published phrase");
}

void testInvalidTimingDoesNotAdoptActiveReplacement()
{
    expect(
        !midigengx::generation::
            shouldAdoptPublishedPhrase(
                true,
                true,
                std::nan(""),
                0.0,
                16.0),
        "invalid current PPQ does not trigger replacement");

    expect(
        !midigengx::generation::
            shouldAdoptPublishedPhrase(
                true,
                true,
                16.0,
                15.0,
                0.0),
        "zero phrase length does not trigger replacement");
}

} // namespace

int main()
{
    testInitialPhraseCanAdoptDuringPlayback();
    testReplacementWaitsForPhraseBoundary();
    testStoppedTransportCanAdoptImmediately();
    testInvalidTimingDoesNotAdoptActiveReplacement();

    if (failures != 0)
        return 1;

    std::cout
        << "MIDI-GenGX Generation activation tests passed.\n";

    return 0;
}
