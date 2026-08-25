#include "Music/MotifKnowledgeCatalog.h"
#include "Music/MotifDevelopment.h"

#include <cstdlib>
#include <iostream>
#include <string>

using namespace midigengx::music;

namespace
{
void expect(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr
            << "FAILED: "
            << message
            << '\n';
        std::exit(1);
    }
}

Motif makeMotif(int anchor = 0)
{
    Motif motif;
    motif.lengthBeats = 2.0;
    motif.notes =
    {
        MotifNote{0.0, 0.5, anchor + 0, 0},
        MotifNote{0.5, 0.5, anchor + 2, 1},
        MotifNote{1.0, 0.5, anchor + 5, -1},
        MotifNote{1.5, 0.5, anchor + 3, 0}
    };
    return motif;
}

MotifKnowledgeCatalog buildCatalog()
{
    const auto base =
        makeMotif();

    const auto distinct =
        MotifDevelopment::retrograde(
            base);

    const auto graph =
        buildMotifOccurrenceGraph(
            {
                base,
                MotifDevelopment::transpose(
                    base,
                    7),
                distinct
            },
            {1, 5, 9});

    const auto profile =
        analyzeMotifRecurrence(
            graph);

    return buildMotifKnowledgeCatalog(
        profile);
}

void testCatalogValidity()
{
    const auto catalog =
        buildCatalog();

    expect(
        catalog.isValid(),
        "knowledge catalog is valid");

    expect(
        catalog.size() == 2,
        "catalog contains unique canonical families");
}

void testDeterministicOrderingAndLookup()
{
    const auto catalog =
        buildCatalog();

    expect(
        catalog.records.size() >= 2,
        "catalog has lookupable records");

    for (std::size_t i = 1;
         i < catalog.records.size();
         ++i)
    {
        expect(
            catalog.records[i - 1].canonicalKey <
                catalog.records[i].canonicalKey,
            "catalog records are strictly canonical-key ordered");
    }

    const auto& key =
        catalog.records.front().canonicalKey;

    const auto* found =
        catalog.findByCanonicalKey(
            key);

    expect(
        found != nullptr,
        "catalog finds canonical key");

    expect(
        found->canonicalKey == key,
        "catalog returns requested record");

    expect(
        catalog.findByCanonicalKey(
            "not-a-real-key") == nullptr,
        "catalog rejects unknown canonical key");
}

void testRecurringStatistics()
{
    const auto catalog =
        buildCatalog();

    expect(
        catalog.recurringCount() == 1,
        "catalog counts recurring families");

    expect(
        catalog.averageOccurrenceCount() >
            1.0,
        "catalog calculates average occurrence count");
}

void testInvalidProfileProducesEmptyCatalog()
{
    MotifRecurrenceProfile invalid;

    const auto catalog =
        buildMotifKnowledgeCatalog(
            invalid);

    expect(
        catalog.records.empty(),
        "invalid profile produces empty catalog");

    expect(
        catalog.isValid(),
        "empty catalog remains a valid result container");
}

} // namespace

int main()
{
    testCatalogValidity();
    testDeterministicOrderingAndLookup();
    testRecurringStatistics();
    testInvalidProfileProducesEmptyCatalog();

    std::cout
        << "MIDI-GenGX Motif Knowledge Catalog tests passed.\n";

    return 0;
}
