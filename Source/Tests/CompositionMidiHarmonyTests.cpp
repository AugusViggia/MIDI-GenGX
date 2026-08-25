#include "Music/CompositionMidiHarmony.h"

#include <array>
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

CompositionMidiCorpusRecord buildCMajorRecord()
{
    CompositionMidiCorpusRecord record;

    record.sampleId =
        "harmony-c-major";

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

        const std::array<int, 3> triad =
        {
            60, 64, 67
        };

        for (int beat = 0;
             beat < 16;
             ++beat)
        {
            for (const auto pitch :
                 triad)
            {
                const auto start =
                    sectionStart +
                    static_cast<std::uint32_t>(
                        beat * 480);

                record.notes.push_back(
                {
                    0,
                    static_cast<std::uint8_t>(
                        pitch),
                    100,
                    start,
                    start + 360
                });
            }
        }
    }

    return record;
}

CompositionMidiSectionAnalysis
buildSections(
    const CompositionMidiCorpusRecord& record)
{
    return analyzeCompositionMidiSections(
        record);
}

void testKeyIsEstimatedAsCMajor()
{
    const auto record =
        buildCMajorRecord();

    const auto sections =
        buildSections(
            record);

    const auto analysis =
        analyzeCompositionMidiHarmony(
            record,
            sections);

    expect(
        analysis.isValid(),
        "C major harmony analysis is valid");

    expect(
        analysis.key.tonicPitchClass == 0,
        "C major tonic is estimated correctly");

    expect(
        analysis.key.scale ==
            CompositionMidiScale::Major,
        "major scale is estimated correctly");
}

void testSectionHarmonyFindsCMajorTriad()
{
    const auto record =
        buildCMajorRecord();

    const auto sections =
        buildSections(
            record);

    const auto analysis =
        analyzeCompositionMidiHarmony(
            record,
            sections);

    expect(
        analysis.sections.size() == 4,
        "four harmonic sections are produced");

    for (const auto& section :
         analysis.sections)
    {
        expect(
            section.valid,
            "section harmony is valid");

        expect(
            section.rootPitchClass == 0,
            "C root is identified");

        expect(
            section.quality ==
                ChordQuality::Major,
            "major chord quality is identified");

        expect(
            section.scaleDegree == 0,
            "C is degree zero in C major");
    }
}

void testHarmonyIsDeterministic()
{
    const auto record =
        buildCMajorRecord();

    const auto sections =
        buildSections(
            record);

    const auto first =
        analyzeCompositionMidiHarmony(
            record,
            sections);

    const auto second =
        analyzeCompositionMidiHarmony(
            record,
            sections);

    expect(
        first.key.tonicPitchClass ==
            second.key.tonicPitchClass &&
        first.key.scale ==
            second.key.scale &&
        first.sections.size() ==
            second.sections.size(),
        "MIDI harmony analysis is deterministic");
}

void testInvalidRecordIsRejected()
{
    CompositionMidiCorpusRecord invalid;
    CompositionMidiSectionAnalysis sections;

    const auto analysis =
        analyzeCompositionMidiHarmony(
            invalid,
            sections);

    expect(
        !analysis.isValid(),
        "invalid MIDI harmony input is rejected");
}

void testAmbiguousSectionDoesNotFabricateUnknownQuality()
{
    auto record =
        buildCMajorRecord();

    for (auto& note :
         record.notes)
    {
        note.velocity =
            static_cast<std::uint8_t>(
                note.velocity / 4);
    }

    const auto sections =
        buildSections(
            record);

    const auto analysis =
        analyzeCompositionMidiHarmony(
            record,
            sections);

    expect(
        analysis.key.isValid(),
        "global key estimate remains valid");

    for (const auto& section :
         analysis.sections)
    {
        expect(
            section.confidence >= 0.0 &&
            section.confidence <= 1.0,
            "section harmony confidence is bounded");
    }
}

} // namespace

int main()
{
    testKeyIsEstimatedAsCMajor();
    testSectionHarmonyFindsCMajorTriad();
    testHarmonyIsDeterministic();
    testInvalidRecordIsRejected();
    testAmbiguousSectionDoesNotFabricateUnknownQuality();

    std::cout
        << "MIDI-GenGX MIDI harmony analysis tests passed.\n";

    return 0;
}
