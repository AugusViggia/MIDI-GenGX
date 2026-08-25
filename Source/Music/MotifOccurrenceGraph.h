#pragma once

#include "MotifRelationship.h"

#include <cstddef>
#include <vector>

namespace midigengx::music
{

struct MotifOccurrence
{
    std::size_t occurrenceIndex = 0;
    std::size_t sourcePhraseIndex = 0;
    double startBeat = 0.0;
    MotifFingerprint fingerprint;
};

struct MotifRelationshipEdge
{
    std::size_t sourceOccurrence = 0;
    std::size_t targetOccurrence = 0;
    MotifRelationship relationship;

    bool isValid() const noexcept;
};

struct MotifOccurrenceGraph
{
    std::vector<MotifOccurrence> occurrences;
    std::vector<MotifRelationshipEdge> edges;
    bool analysisValid = false;

    bool isValid() const noexcept;

    std::size_t countOccurrencesOf(
        const MotifFingerprint& fingerprint) const noexcept;
};

MotifOccurrenceGraph buildMotifOccurrenceGraph(
    const std::vector<Motif>& motifs,
    const std::vector<std::size_t>& sourcePhraseIndices = {}) noexcept;

} // namespace midigengx::music
