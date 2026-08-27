#include "Music/CompositionMidiSectionAnalyzer.h"

#include <cstdlib>
#include <iostream>

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
        "sections";

    record.ticksPerQuarterNote =
        480;

    record.trackCount =
        1;

    record.lengthTicks =
        16 * 480;

    record.analysisValid =
        true;

    for (int index = 0;
         index < 16;
         ++index)
    {
        const auto start =
            static_cast<std::uint32_t>(
                index * 480);

        record.notes.push_back(
        {
            0,
            static_cast<std::uint8_t>(
                60 + index % 5),
            static_cast<std::uint8_t>(
                80 + index % 20),
            start,
            start + 240
        });
    }

    return record;
}

void testFourBarSegmentation()
{
    const auto record =
        buildRecord();

    const auto analysis =
        analyzeCompositionMidiSections(
            record);

    expect(
        analysis.isValid(
            record.ticksPerQuarterNote),
        "segmentation analysis is valid");

    expect(
        analysis.sectionCount() == 1,
        "16-quarter-note fixture forms one minimum four-bar section");
}

void testSixteenBarStructureProducesFourSections()
{
    auto record =
        buildRecord();

    record.lengthTicks =
        64 * 480;

    record.notes.clear();

    for (int section = 0;
         section < 4;
         ++section)
    {
        for (int note = 0;
             note < 16;
             ++note)
        {
            const auto tick =
                static_cast<std::uint32_t>(
                    section * 16 * 480 +
                    note * 480);

            record.notes.push_back(
            {
                0,
                static_cast<std::uint8_t>(
                    60 + section),
                90,
                tick,
                tick + 240
            });
        }
    }

    const auto analysis =
        analyzeCompositionMidiSections(
            record);

    expect(
        analysis.isValid(
            record.ticksPerQuarterNote),
        "four-section analysis is valid");

    expect(
        analysis.sectionCount() == 4,
        "16-bar MIDI is segmented into four four-bar sections");

    expect(
        analysis.sections.front().role ==
            PhraseSection::Opening,
        "first section is opening");

    expect(
        analysis.sections.back().role ==
            PhraseSection::Cadence,
        "last section is cadence");

    expect(
        analysis.sections[1].role ==
            PhraseSection::Development &&
        analysis.sections[2].role ==
            PhraseSection::Development,
        "middle sections are development");
}

void testSectionBoundariesAreContiguous()
{
    const auto record =
        buildRecord();

    auto extended =
        record;

    extended.lengthTicks =
        32 * 480;

    extended.notes.clear();

    for (int index = 0;
         index < 32;
         ++index)
    {
        const auto start =
            static_cast<std::uint32_t>(
                index * 480);

        extended.notes.push_back(
        {
            0,
            64,
            96,
            start,
            start + 120
        });
    }

    const auto analysis =
        analyzeCompositionMidiSections(
            extended);

    expect(
        analysis.isValid(
            extended.ticksPerQuarterNote),
        "contiguous section analysis is valid");

    for (std::size_t index = 1;
         index < analysis.sections.size();
         ++index)
    {
        expect(
            analysis.sections[index - 1].endTick ==
                analysis.sections[index].startTick,
            "section boundaries are contiguous");
    }
}

void testEmptySectionIsNotCreated()
{
    auto record =
        buildRecord();

    record.lengthTicks =
        32 * 480;

    record.notes.clear();

    for (int index = 0;
         index < 16;
         ++index)
    {
        const auto start =
            static_cast<std::uint32_t>(
                index * 480);

        record.notes.push_back(
        {
            0,
            60,
            90,
            start,
            start + 120
        });
    }

    const auto analysis =
        analyzeCompositionMidiSections(
            record);

    expect(
        analysis.isValid(
            record.ticksPerQuarterNote),
        "sparse section analysis remains valid");

    expect(
        analysis.sectionCount() == 1,
        "empty four-bar regions are omitted");
}

void testSectionMetricsPropagate()
{
    auto record =
        buildRecord();

    record.lengthTicks =
        64 * 480;

    record.notes.clear();

    for (int index = 0;
         index < 64;
         ++index)
    {
        const auto start =
            static_cast<std::uint32_t>(
                index * 480);

        const auto dense =
            index >= 32;

        record.notes.push_back(
        {
            0,
            static_cast<std::uint8_t>(
                dense ? 72 : 60),
            static_cast<std::uint8_t>(
                dense ? 120 : 60),
            start,
            start +
                static_cast<std::uint32_t>(
                    dense ? 420 : 120)
        });

        if (dense)
        {
            record.notes.push_back(
            {
                0,
                67,
                100,
                start,
                start + 240
            });
        }
    }

    const auto analysis =
        analyzeCompositionMidiSections(
            record);

    expect(
        analysis.isValid(
            record.ticksPerQuarterNote),
        "metric propagation analysis is valid");

    expect(
        analysis.sections.size() == 4,
        "metric propagation retains four sections");

    expect(
        analysis.sections.back().notesPerBeat >
            analysis.sections.front().notesPerBeat,
        "density changes propagate into section metrics");

    expect(
        analysis.sections.back().tension >
            analysis.sections.front().tension,
        "density/velocity changes propagate into tension");
}

void testShortTerminalSectionIsAccepted()
{
    auto record =
        buildRecord();

    record.lengthTicks =
        17 * 480;

    record.notes.clear();

    for (int index = 0;
         index < 17;
         ++index)
    {
        const auto start =
            static_cast<std::uint32_t>(
                index * 480);

        record.notes.push_back(
        {
            0,
            static_cast<std::uint8_t>(
                60 + index % 5),
            90,
            start,
            start + 240
        });
    }

    const auto analysis =
        analyzeCompositionMidiSections(
            record);

    expect(
        analysis.isValid(
            record.ticksPerQuarterNote),
        "short terminal section remains valid");

    expect(
        analysis.sectionCount() == 2,
        "17-quarter-note fixture produces a terminal section");

    expect(
        analysis.sections.back().role ==
            PhraseSection::Cadence,
        "short terminal section is cadence");

    const auto terminalLengthBeats =
        static_cast<double>(
            analysis.sections.back().endTick -
            analysis.sections.back().startTick) /
        static_cast<double>(
            record.ticksPerQuarterNote);

    expect(
        terminalLengthBeats == 1.0,
        "terminal section preserves its one-beat duration");
}

void testInvalidRecordIsRejected()
{
    CompositionMidiCorpusRecord invalid;

    const auto analysis =
        analyzeCompositionMidiSections(
            invalid);

    expect(
        !analysis.isValid(480),
        "invalid MIDI record is rejected");
}

} // namespace

int main()
{
    testFourBarSegmentation();
    testSixteenBarStructureProducesFourSections();
    testSectionBoundariesAreContiguous();
    testEmptySectionIsNotCreated();
    testSectionMetricsPropagate();
    testShortTerminalSectionIsAccepted();
    testInvalidRecordIsRejected();

    std::cout
        << "MIDI-GenGX MIDI section analyzer tests passed.\n";

    return 0;
}
