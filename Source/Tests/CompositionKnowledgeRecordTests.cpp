#include "Music/CompositionKnowledgeRecord.h"
#include "Music/MotifDevelopment.h"

#include <cstdlib>
#include <iostream>
#include <vector>

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

CompositionKnowledgeRecord buildRecord()
{
    midigengx::domain::MusicalContext context;
    context.parameters.lengthBars = 16;
    context.parameters.phraseLengthBars = 4;
    context.normalize();

    const auto structure =
        planPhraseStructure(context);

    const auto harmony =
        planHarmony(
            context,
            structure);

    const auto graph =
        buildMotifOccurrenceGraph(
            {
                makeMotif(),
                MotifDevelopment::transpose(
                    makeMotif(),
                    7)
            },
            {0, 8});

    const auto profile =
        analyzeMotifRecurrence(
            graph);

    const auto catalog =
        buildMotifKnowledgeCatalog(
            profile);

    return buildCompositionKnowledgeRecord(
        structure,
        harmony,
        catalog);
}

void testRecordValidityAndStructure()
{
    const auto record =
        buildRecord();

    expect(
        record.isValid(),
        "composition knowledge record is valid");

    expect(
        record.totalLengthBeats == 64.0,
        "composition length is represented in beats");

    expect(
        record.sectionCount == 4,
        "composition section count is represented");

    expect(
        record.harmonyEventCount == 4,
        "harmony event count matches sections");
}

void testTensionSummary()
{
    const auto record =
        buildRecord();

    expect(
        record.sectionTensions.size() ==
            record.sectionCount,
        "section tension vector matches section count");

    expect(
        record.averageSectionTension >=
            record.minimumSectionTension &&
        record.averageSectionTension <=
            record.maximumSectionTension,
        "average tension lies within section bounds");

    expect(
        record.minimumSectionTension >= 0 &&
        record.maximumSectionTension <= 100,
        "tension summary remains normalized");
}

void testMotifSummary()
{
    const auto record =
        buildRecord();

    expect(
        record.totalMotifFamilyCount == 1,
        "transposed motif belongs to one knowledge family");

    expect(
        record.recurringMotifFamilyCount == 1,
        "composition records recurring motif families");

    expect(
        record.averageMotifOccurrences == 2.0,
        "composition records average motif occurrences");
}

void testHarmonySummary()
{
    const auto record =
        buildRecord();

    const auto knownHarmony =
        record.majorHarmonyEvents +
        record.minorHarmonyEvents +
        record.diminishedHarmonyEvents +
        record.augmentedHarmonyEvents +
        record.suspendedHarmonyEvents +
        record.unknownHarmonyEvents;

    expect(
        knownHarmony ==
            record.harmonyEventCount,
        "harmony quality counters cover every event");
}

void testMismatchedCompositionLengthIsRejected()
{
    midigengx::domain::MusicalContext context;
    context.parameters.lengthBars = 16;
    context.parameters.phraseLengthBars = 4;
    context.normalize();

    const auto structure =
        planPhraseStructure(context);

    auto harmony =
        planHarmony(
            context,
            structure);

    harmony.totalLengthBeats += 1.0;

    const auto graph =
        buildMotifOccurrenceGraph(
            {makeMotif()},
            {0});

    const auto profile =
        analyzeMotifRecurrence(
            graph);

    const auto catalog =
        buildMotifKnowledgeCatalog(
            profile);

    const auto record =
        buildCompositionKnowledgeRecord(
            structure,
            harmony,
            catalog);

    expect(
        !record.isValid(),
        "mismatched structural and harmony lengths are rejected");
}

} // namespace

int main()
{
    testRecordValidityAndStructure();
    testTensionSummary();
    testMotifSummary();
    testHarmonySummary();
    testMismatchedCompositionLengthIsRejected();

    std::cout
        << "MIDI-GenGX Composition Knowledge Record tests passed.\n";

    return 0;
}
