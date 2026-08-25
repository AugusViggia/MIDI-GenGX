#pragma once

namespace midigengx::generation
{

// Pure transport/phrase-boundary policy. It deliberately contains no JUCE
// types so the timing contract can be unit-tested independently.
bool shouldAdoptPublishedPhrase(
    bool hasActivePhrase,
    bool transportPlaying,
    double currentPpq,
    double previousPpq,
    double phraseLengthBeats) noexcept;

} // namespace midigengx::generation
