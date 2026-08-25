#include "Music/CompositionMidiMotifAnalysis.h"

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
        "motif-analysis";

    record.ticksPerQuarterNote =
        480;

    record.trackCount =
        1;

    record.lengthTicks =
        64 * 480;

    record.analysisValid =
        true;

    for (int section = 0;
         section < 4;
         ++section)
    {
        const auto sectionStart =
            static_cast<std::uint32_t>(
                section * 16 * 480);

        const int base =
            60;

        const int intervalPattern[4] =
        {
            0, 2, 4, 2
        };

        for (int index = 0;
             index < 16;
             ++index)
        {
            const auto start =
                sectionStart +
                static_cast<std::uint32_t>(
                    index * 480);

            const int pitch =
                base +
                intervalPattern[
                    index % 4];

            record.notes.push_back(
            {
                0,
                static_cast<std::uint8_t>(
                    pitch),
                96,
                start,
                start + 240
            });
        }
    }

    return record;
}

void testMotifAnalysisIsValid()
{
    const auto record =
        buildRecord();

    const auto sections =
        analyzeCompositionMidiSections(
            record);

    const auto analysis =
        analyzeCompositionMidiMotifs(
            record,
            sections);

    expect(
        analysis.isValid(),
        "MIDI motif analysis is valid");
}

void testMotifFamiliesAreDetected()
{
    const auto record =
        buildRecord();

    const auto sections =
        analyzeCompositionMidiSections(
            record);

    const auto analysis =
        analyzeCompositionMidiMotifs(
            record,
            sections);

    expect(
        analysis.totalFamilyCount() > 0,
        "at least one motif family is detected");

    expect(
        analysis.recurringFamilyCount() > 0,
        "repeated section motifs create recurring families");

    expect(
        analysis.averageOccurrenceCount() >= 1.0,
        "motif occurrence statistics are populated");
}

void testMotifAnalysisIsDeterministic()
{
    const auto record =
        buildRecord();

    const auto sections =
        analyzeCompositionMidiSections(
            record);

    const auto first =
        analyzeCompositionMidiMotifs(
            record,
            sections);

    const auto second =
        analyzeCompositionMidiMotifs(
            record,
            sections);

    expect(
        first.catalog.size() ==
            second.catalog.size(),
        "motif family count is deterministic");

    expect(
        first.recurringFamilyCount() ==
            second.recurringFamilyCount(),
        "recurring motif count is deterministic");

    expect(
        first.averageOccurrenceCount() ==
            second.averageOccurrenceCount(),
        "motif occurrence average is deterministic");
}

void testInvalidInputIsRejected()
{
    CompositionMidiCorpusRecord invalid;
    CompositionMidiSectionAnalysis sections;

    const auto analysis =
        analyzeCompositionMidiMotifs(
            invalid,
            sections);

    expect(
        !analysis.isValid(),
        "invalid MIDI motif input is rejected");
}

} // namespace

int main()
{
    testMotifAnalysisIsValid();
    testMotifFamiliesAreDetected();
    testMotifAnalysisIsDeterministic();
    testInvalidInputIsRejected();

    std::cout
        << "MIDI-GenGX MIDI motif analysis tests passed.\n";

    return 0;
}
