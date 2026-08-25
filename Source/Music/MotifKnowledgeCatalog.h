#pragma once

#include "MotifKnowledgeRecord.h"

#include <cstddef>
#include <string>
#include <vector>

namespace midigengx::music
{

struct MotifKnowledgeCatalog
{
    std::vector<MotifKnowledgeRecord> records;

    bool isValid() const noexcept;

    std::size_t size() const noexcept;

    const MotifKnowledgeRecord* findByCanonicalKey(
        const std::string& canonicalKey) const noexcept;

    std::size_t recurringCount() const noexcept;

    double averageOccurrenceCount() const noexcept;
};

MotifKnowledgeCatalog buildMotifKnowledgeCatalog(
    const MotifRecurrenceProfile& profile) noexcept;

} // namespace midigengx::music
