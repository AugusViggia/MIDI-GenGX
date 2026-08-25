#include "Music/CompositionMidiTrainingSequence.h"

#include <cstdlib>
#include <iostream>
#include <vector>

using namespace midigengx::music;

namespace
{

void expect(
    bool condition,
    const char* message)
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

CompositionMidiCorpusRecord buildRecord()
{
    CompositionMidiCorpusRecord record;

    record.sampleId =
        "phase83-sequence";

    record.ticksPerQuarterNote =
        480;

    record.trackCount =
        1;

    record.lengthTicks =
        64 * 480;

    record.analysisValid =
        true;

    const int pitches[4] =
    {
        60, 64, 67, 64
    };

    for (int index = 0;
         index < 64;
         ++index)
    {
        const auto start =
            static_cast<std::uint32_t>(
                index * 480);

        record.notes.push_back(
        {
            0,
            static_cast<std::uint8_t>(
                pitches[index % 4]),
            static_cast<std::uint8_t>(
                80 +
                (index % 32)),
            start,
            start + 360
        });
    }

    return record;
}

void buildAnalysis(
    const CompositionMidiCorpusRecord& record,
    CompositionMidiSectionAnalysis& sections,
    CompositionMidiHarmonyAnalysis& harmony,
    CompositionMidiMotifAnalysis& motifs)
{
    sections =
        analyzeCompositionMidiSections(
            record);

    expect(
        sections.isValid(
            record.ticksPerQuarterNote),
        "section analysis is valid");

    harmony =
        analyzeCompositionMidiHarmony(
            record,
            sections);

    expect(
        harmony.isValid(),
        "harmony analysis is valid");

    motifs =
        analyzeCompositionMidiMotifs(
            record,
            sections);

    expect(
        motifs.isValid(),
        "motif analysis is valid");
}

void testSequenceIsValid()
{
    const auto record =
        buildRecord();

    CompositionMidiSectionAnalysis sections;
    CompositionMidiHarmonyAnalysis harmony;
    CompositionMidiMotifAnalysis motifs;

    buildAnalysis(
        record,
        sections,
        harmony,
        motifs);

    const auto sequence =
        buildCompositionMidiTrainingSequence(
            record,
            sections,
            harmony,
            motifs);

    expect(
        sequence.isValid(),
        "Phase 83 training sequence is valid");

    expect(
        sequence.eventCount() ==
            record.noteCount(),
        "one training event is produced per MIDI note");
}

void testFeatureWidthIsStable()
{
    const auto record =
        buildRecord();

    CompositionMidiSectionAnalysis sections;
    CompositionMidiHarmonyAnalysis harmony;
    CompositionMidiMotifAnalysis motifs;

    buildAnalysis(
        record,
        sections,
        harmony,
        motifs);

    const auto sequence =
        buildCompositionMidiTrainingSequence(
            record,
            sections,
            harmony,
            motifs);

    expect(
        sequence.featureWidth ==
            CompositionMidiTrainingEvent::featureCount,
        "event feature width matches schema");

    expect(
        sequence.featureWidth == 20,
        "Phase 83 event feature schema contains 20 features");
}

void testAllFeaturesAreBounded()
{
    const auto record =
        buildRecord();

    CompositionMidiSectionAnalysis sections;
    CompositionMidiHarmonyAnalysis harmony;
    CompositionMidiMotifAnalysis motifs;

    buildAnalysis(
        record,
        sections,
        harmony,
        motifs);

    const auto sequence =
        buildCompositionMidiTrainingSequence(
            record,
            sections,
            harmony,
            motifs);

    for (const auto& event :
         sequence.events)
    {
        expect(
            event.isValid(),
            "every event contains finite bounded ML features");
    }
}

void testSequenceIsDeterministic()
{
    const auto record =
        buildRecord();

    CompositionMidiSectionAnalysis sections;
    CompositionMidiHarmonyAnalysis harmony;
    CompositionMidiMotifAnalysis motifs;

    buildAnalysis(
        record,
        sections,
        harmony,
        motifs);

    const auto first =
        buildCompositionMidiTrainingSequence(
            record,
            sections,
            harmony,
            motifs);

    const auto second =
        buildCompositionMidiTrainingSequence(
            record,
            sections,
            harmony,
            motifs);

    expect(
        first.isValid() &&
        second.isValid(),
        "deterministic sequence builds are valid");

    expect(
        first.events.size() ==
            second.events.size(),
        "deterministic sequence preserves event count");

    for (std::size_t index = 0;
         index < first.events.size();
         ++index)
    {
        expect(
            first.events[index].features ==
                second.events[index].features,
            "deterministic sequence features are identical");
    }
}

void testInvalidMidiIsRejected()
{
    CompositionMidiCorpusRecord record;

    CompositionMidiSectionAnalysis sections;
    CompositionMidiHarmonyAnalysis harmony;
    CompositionMidiMotifAnalysis motifs;

    const auto sequence =
        buildCompositionMidiTrainingSequence(
            record,
            sections,
            harmony,
            motifs);

    expect(
        !sequence.isValid(),
        "invalid MIDI sequence input is rejected");
}

} // namespace

int main()
{
    testSequenceIsValid();
    testFeatureWidthIsStable();
    testAllFeaturesAreBounded();
    testSequenceIsDeterministic();
    testInvalidMidiIsRejected();

    std::cout
        << "MIDI-GenGX Phase 83 training sequence tests passed.\n";

    return 0;
}
