#include "Music/MotifRecurrenceMetrics.h"
#include "Music/MotifDevelopment.h"

#include <cstdlib>
#include <iostream>

using namespace midigengx::music;

namespace
{
void expect(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\n';
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

void testSingleOccurrence()
{
    const auto graph =
        buildMotifOccurrenceGraph(
            {makeMotif()},
            {4});

    const auto profile =
        analyzeMotifRecurrence(graph);

    const auto metrics =
        calculateMotifRecurrenceMetrics(
            &profile.families.front());

    expect(
        metrics.isValid(),
        "single occurrence metrics are valid");

    expect(
        metrics.pattern ==
            MotifRecurrencePattern::Single,
        "single occurrence is classified as single");

    expect(
        metrics.occurrenceCount == 1,
        "single occurrence count is one");

    expect(
        metrics.phraseGaps.empty(),
        "single occurrence has no phrase gaps");
}

void testPeriodicRecurrence()
{
    const auto base =
        makeMotif();

    const auto graph =
        buildMotifOccurrenceGraph(
            {
                base,
                MotifDevelopment::transpose(base, 4),
                MotifDevelopment::transpose(base, 7),
                MotifDevelopment::transpose(base, 9)
            },
            {0, 4, 8, 12});

    const auto profile =
        analyzeMotifRecurrence(graph);

    const auto metrics =
        calculateMotifRecurrenceMetrics(
            &profile.families.front());

    expect(
        metrics.isValid(),
        "periodic recurrence metrics are valid");

    expect(
        metrics.pattern ==
            MotifRecurrencePattern::Periodic,
        "equal phrase gaps create periodic recurrence");

    expect(
        metrics.averagePhraseGap == 4.0,
        "periodic recurrence has correct average gap");

    expect(
        metrics.phraseGaps ==
            std::vector<std::size_t>({4, 4, 4}),
        "periodic recurrence stores exact phrase gaps");
}

void testClusteredRecurrence()
{
    const auto base =
        makeMotif();

    const auto graph =
        buildMotifOccurrenceGraph(
            {
                base,
                MotifDevelopment::transpose(base, 2),
                MotifDevelopment::transpose(base, 5),
                MotifDevelopment::transpose(base, 7)
            },
            {0, 1, 2, 7});

    const auto profile =
        analyzeMotifRecurrence(graph);

    const auto metrics =
        calculateMotifRecurrenceMetrics(
            &profile.families.front());

    expect(
        metrics.isValid(),
        "clustered recurrence metrics are valid");

    expect(
        metrics.pattern ==
            MotifRecurrencePattern::Clustered,
        "tight phrase spacing creates clustered recurrence");

    expect(
        std::abs(
            metrics.recurrenceDensity -
            (3.0 / 7.0)) < 1.0e-9,
        "clustered recurrence density matches phrase span");
}

void testTransformationRate()
{
    const auto base =
        makeMotif();

    const auto transposed =
        MotifDevelopment::transpose(base, 5);

    const auto graph =
        buildMotifOccurrenceGraph(
            {base, transposed},
            {3, 9});

    const auto profile =
        analyzeMotifRecurrence(graph);

    const auto metrics =
        calculateMotifRecurrenceMetrics(
            &profile.families.front());

    expect(
        metrics.transformationRate > 0.0,
        "recurrence family exposes transformation rate");

    expect(
        metrics.transformationRate <= 1.0,
        "transformation rate is normalized");
}

void testNullFamily()
{
    const auto metrics =
        calculateMotifRecurrenceMetrics(
            nullptr);

    expect(
        metrics.isValid(),
        "null recurrence family produces valid absent metrics");

    expect(
        metrics.pattern ==
            MotifRecurrencePattern::Absent &&
        metrics.occurrenceCount == 0,
        "null recurrence family is represented as absent");
}

} // namespace

int main()
{
    testSingleOccurrence();
    testPeriodicRecurrence();
    testClusteredRecurrence();
    testTransformationRate();
    testNullFamily();

    std::cout
        << "MIDI-GenGX Motif Recurrence Metrics tests passed.\n";

    return 0;
}
