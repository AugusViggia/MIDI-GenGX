#pragma once

#include "MotifOccurrenceGraph.h"

#include <cstddef>
#include <vector>

namespace midigengx::music
{

struct MotifRecurrenceFamily
{
    std::size_t familyIndex = 0;
    MotifFingerprint fingerprint;

    std::vector<std::size_t> occurrenceIndices;
    std::vector<std::size_t> phraseIndices;

    std::size_t firstPhraseIndex = 0;
    std::size_t lastPhraseIndex = 0;

    std::size_t transpositionCount = 0;
    std::size_t retrogradeCount = 0;
    std::size_t inversionCount = 0;
    std::size_t rhythmicVariationCount = 0;
    std::size_t intervalVariationCount = 0;
    std::size_t compoundVariationCount = 0;

    bool isRecurring() const noexcept;
    std::size_t transformationCount() const noexcept;
};

struct MotifRecurrenceProfile
{
    std::vector<MotifRecurrenceFamily> families;
    bool analysisValid = false;

    bool isValid() const noexcept;

    const MotifRecurrenceFamily* findFamily(
        const MotifFingerprint& fingerprint) const noexcept;
};

MotifRecurrenceProfile analyzeMotifRecurrence(
    const MotifOccurrenceGraph& graph) noexcept;

} // namespace midigengx::music
