#include "MotifFingerprint.h"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace midigengx::music
{
namespace
{

int quantizeRhythm(
    double value) noexcept
{
    // Musical subdivisions are represented in eighth-beat units. The
    // quantization is intentionally small and deterministic so fingerprints
    // are stable across tiny floating-point differences.
    return static_cast<int>(
        std::lround(
            value * 8.0));
}

int contourDirection(
    int previous,
    int current) noexcept
{
    if (current > previous)
        return 1;

    if (current < previous)
        return -1;

    return 0;
}

} // namespace

bool MotifFingerprint::isValid() const noexcept
{
    const std::size_t expectedIntervals =
        noteCount > 0
            ? noteCount - 1
            : 0;

    return noteCount > 0 &&
           intervalSignature.size() ==
               expectedIntervals &&
           rhythmSignature.size() ==
               noteCount &&
           contourSignature.size() ==
               expectedIntervals;
}

std::string MotifFingerprint::canonicalKey() const
{
    if (!isValid())
        return {};

    std::ostringstream stream;

    stream << "I:";
    for (const int interval :
         intervalSignature)
    {
        stream << interval << ',';
    }

    stream << "|R:";
    for (const int rhythm :
         rhythmSignature)
    {
        stream << rhythm << ',';
    }

    stream << "|C:";
    for (const int contour :
         contourSignature)
    {
        stream << contour << ',';
    }

    return stream.str();
}

MotifFingerprint fingerprintMotif(
    const Motif& motif) noexcept
{
    MotifFingerprint result;

    if (!motif.isValid() ||
        motif.notes.empty())
    {
        return result;
    }

    result.noteCount =
        motif.notes.size();

    result.lengthBeats =
        motif.lengthBeats;

    result.firstPitchAnchor =
        motif.notes.front().relativePitch;

    result.rhythmSignature.reserve(
        motif.notes.size());

    if (motif.notes.size() > 1)
    {
        result.intervalSignature.reserve(
            motif.notes.size() - 1);

        result.contourSignature.reserve(
            motif.notes.size() - 1);
    }

    for (const auto& note :
         motif.notes)
    {
        result.rhythmSignature.push_back(
            quantizeRhythm(
                note.durationBeats));
    }

    for (std::size_t i = 1;
         i < motif.notes.size();
         ++i)
    {
        const auto& previous =
            motif.notes[i - 1];

        const auto& current =
            motif.notes[i];

        result.intervalSignature.push_back(
            current.relativePitch -
            previous.relativePitch);

        result.contourSignature.push_back(
            contourDirection(
                previous.relativePitch,
                current.relativePitch));
    }

    return result;
}

bool isMotivicIdentityEquivalent(
    const MotifFingerprint& a,
    const MotifFingerprint& b) noexcept
{
    if (!a.isValid() ||
        !b.isValid())
    {
        return false;
    }

    return a.intervalSignature ==
               b.intervalSignature &&
           a.rhythmSignature ==
               b.rhythmSignature &&
           a.contourSignature ==
               b.contourSignature;
}

} // namespace midigengx::music
