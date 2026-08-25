#include "MotifKnowledgeCatalog.h"

#include <algorithm>
#include <numeric>

namespace midigengx::music
{

bool MotifKnowledgeCatalog::isValid() const noexcept
{
    std::string previousKey;

    for (const auto& record :
         records)
    {
        if (!record.isValid() ||
            record.canonicalKey.empty())
        {
            return false;
        }

        // Catalog records are canonical-key ordered and unique. This gives
        // deterministic dataset serialization and stable lookup behavior.
        if (!previousKey.empty() &&
            record.canonicalKey <= previousKey)
        {
            return false;
        }

        previousKey =
            record.canonicalKey;
    }

    return true;
}

std::size_t MotifKnowledgeCatalog::size() const noexcept
{
    return records.size();
}

const MotifKnowledgeRecord*
MotifKnowledgeCatalog::findByCanonicalKey(
    const std::string& canonicalKey) const noexcept
{
    if (canonicalKey.empty())
        return nullptr;

    const auto it =
        std::lower_bound(
            records.begin(),
            records.end(),
            canonicalKey,
            [](const MotifKnowledgeRecord& record,
               const std::string& key)
            {
                return record.canonicalKey < key;
            });

    if (it == records.end() ||
        it->canonicalKey != canonicalKey)
    {
        return nullptr;
    }

    return &(*it);
}

std::size_t MotifKnowledgeCatalog::recurringCount() const noexcept
{
    return static_cast<std::size_t>(
        std::count_if(
            records.begin(),
            records.end(),
            [](const MotifKnowledgeRecord& record)
            {
                return record.isRecurring();
            }));
}

double MotifKnowledgeCatalog::averageOccurrenceCount() const noexcept
{
    if (records.empty())
        return 0.0;

    const auto total =
        std::accumulate(
            records.begin(),
            records.end(),
            std::size_t { 0 },
            [](std::size_t sum,
               const MotifKnowledgeRecord& record)
            {
                return sum +
                       record.occurrenceCount;
            });

    return static_cast<double>(
               total) /
           static_cast<double>(
               records.size());
}

MotifKnowledgeCatalog buildMotifKnowledgeCatalog(
    const MotifRecurrenceProfile& profile) noexcept
{
    MotifKnowledgeCatalog catalog;

    if (!profile.analysisValid ||
        !profile.isValid())
    {
        return catalog;
    }

    catalog.records.reserve(
        profile.families.size());

    for (const auto& family :
         profile.families)
    {
        const auto metrics =
            calculateMotifRecurrenceMetrics(
                &family);

        const auto record =
            buildMotifKnowledgeRecord(
                family,
                metrics);

        if (record.isValid())
            catalog.records.push_back(
                record);
    }

    std::sort(
        catalog.records.begin(),
        catalog.records.end(),
        [](const MotifKnowledgeRecord& a,
           const MotifKnowledgeRecord& b)
        {
            return a.canonicalKey <
                   b.canonicalKey;
        });

    catalog.records.erase(
        std::unique(
            catalog.records.begin(),
            catalog.records.end(),
            [](const MotifKnowledgeRecord& a,
               const MotifKnowledgeRecord& b)
            {
                return a.canonicalKey ==
                       b.canonicalKey;
            }),
        catalog.records.end());

    return catalog;
}

} // namespace midigengx::music
