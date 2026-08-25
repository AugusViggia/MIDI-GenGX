#include "Music/MotifKnowledgeRecord.h"
#include "Music/MotifDevelopment.h"

#include <cmath>
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

MotifKnowledgeRecord makeRecord()
{
    const auto base = makeMotif();

    const auto graph =
        buildMotifOccurrenceGraph(
            {
                base,
                MotifDevelopment::transpose(base, 7),
                base
            },
            {1, 5, 9});

    const auto profile =
        analyzeMotifRecurrence(graph);

    const auto metrics =
        calculateMotifRecurrenceMetrics(
            &profile.families.front());

    return buildMotifKnowledgeRecord(
        profile.families.front(),
        metrics);
}

void testRecordCapturesStableIdentity()
{
    const auto record =
        makeRecord();

    expect(
        record.isValid(),
        "knowledge record is valid");

    expect(
        !record.canonicalKey.empty(),
        "knowledge record contains canonical identity");

    expect(
        record.noteCount == 4 &&
        record.lengthBeats == 2.0,
        "knowledge record preserves motif structure");
}

void testRecordCapturesRecurrence()
{
    const auto record =
        makeRecord();

    expect(
        record.isRecurring(),
        "knowledge record identifies recurrence");

    expect(
        record.occurrenceCount == 3,
        "knowledge record preserves occurrence count");

    expect(
        record.firstPhraseIndex == 1 &&
        record.lastPhraseIndex == 9,
        "knowledge record preserves phrase span");

    expect(
        record.recurrenceDensity > 0.0 &&
        record.recurrenceDensity <= 1.0,
        "knowledge record preserves normalized recurrence density");
}

void testRecordCapturesTransformations()
{
    const auto record =
        makeRecord();

    expect(
        record.transpositionCount >= 1,
        "knowledge record preserves transposition count");

    expect(
        record.transformationRate > 0.0 &&
        record.transformationRate <= 1.0,
        "knowledge record preserves normalized transformation rate");
}

void testSingleRecord()
{
    const auto motif = makeMotif();

    const auto graph =
        buildMotifOccurrenceGraph(
            {motif},
            {6});

    const auto profile =
        analyzeMotifRecurrence(graph);

    const auto metrics =
        calculateMotifRecurrenceMetrics(
            &profile.families.front());

    const auto record =
        buildMotifKnowledgeRecord(
            profile.families.front(),
            metrics);

    expect(
        record.isValid(),
        "single occurrence knowledge record is valid");

    expect(
        record.recurrencePattern ==
            MotifRecurrencePattern::Single,
        "single occurrence record retains pattern");

    expect(
        !record.isRecurring(),
        "single occurrence record is not recurring");
}

void testInvalidInputsProduceInvalidRecord()
{
    MotifRecurrenceFamily invalidFamily;
    MotifRecurrenceMetrics invalidMetrics;

    const auto record =
        buildMotifKnowledgeRecord(
            invalidFamily,
            invalidMetrics);

    expect(
        !record.isValid(),
        "invalid knowledge inputs produce invalid record");
}

} // namespace

int main()
{
    testRecordCapturesStableIdentity();
    testRecordCapturesRecurrence();
    testRecordCapturesTransformations();
    testSingleRecord();
    testInvalidInputsProduceInvalidRecord();

    std::cout
        << "MIDI-GenGX Motif Knowledge Record tests passed.\n";

    return 0;
}
