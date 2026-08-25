#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace midigengx::music
{

struct CompositionSequenceMetadata
{
    static constexpr int version = 1;

    std::string sampleId;

    std::string composerId;
    std::string workId;
    std::string movementId;

    std::string styleId;
    std::string eraId;
    std::string instrumentationId;

    bool verified = false;
    bool valid = false;

    bool isValid() const noexcept;
};

} // namespace midigengx::music
