#pragma once

#include "Motif.h"

#include <cstdint>
#include <string>
#include <vector>

namespace midigengx::music
{

struct MotifFingerprint
{
    std::vector<int> intervalSignature;
    std::vector<int> rhythmSignature;
    std::vector<int> contourSignature;

    int firstPitchAnchor = 0;

    double lengthBeats = 0.0;
    std::size_t noteCount = 0;

    bool isValid() const noexcept;

    // Canonical identity excludes absolute pitch and represents the motif
    // through relative interval/rhythm/contour information.
    std::string canonicalKey() const;
};

MotifFingerprint fingerprintMotif(
    const Motif& motif) noexcept;

bool isMotivicIdentityEquivalent(
    const MotifFingerprint& a,
    const MotifFingerprint& b) noexcept;

} // namespace midigengx::music
