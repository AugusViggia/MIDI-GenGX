#pragma once

#include "CompositionConditionedTrainingSample.h"

#include <cstdint>
#include <string>
#include <vector>

namespace midigengx::music
{

struct CompositionConditioningVocabulary
{
    static constexpr int version = 1;

    std::vector<std::string> composers;
    std::vector<std::string> styles;
    std::vector<std::string> eras;
    std::vector<std::string> instrumentations;

    bool valid = false;

    bool isValid() const noexcept;

    std::uint32_t composerIndex(
        const std::string& value) const noexcept;

    std::uint32_t styleIndex(
        const std::string& value) const noexcept;

    std::uint32_t eraIndex(
        const std::string& value) const noexcept;

    std::uint32_t instrumentationIndex(
        const std::string& value) const noexcept;
};

CompositionConditioningVocabulary
buildCompositionConditioningVocabulary(
    const std::vector<CompositionSequenceMetadata>& metadata) noexcept;

} // namespace midigengx::music
