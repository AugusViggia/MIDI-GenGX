#include "Music/CompositionMidiCorpusAnalysis.h"

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
        "analysis";

    record.ticksPerQuarterNote =
        480;

    record.trackCount =
        1;

    record.lengthTicks =
        960;

    record.notes =
    {
        CompositionMidiNote{
            0, 60, 100,
            0, 480},

        CompositionMidiNote{
            0, 64, 80,
            0, 480},

        CompositionMidiNote{
            0, 67, 90,
            480, 960}
    };

    record.analysisValid =
        true;

    return record;
}

void testAnalysisIsValid()
{
    const auto record =
        buildRecord();

    const auto analysis =
        analyzeCompositionMidiCorpus(
            record);

    expect(
        analysis.isValid(),
        "MIDI corpus analysis is valid");
}

void testBasicMetrics()
{
    const auto record =
        buildRecord();

    const auto analysis =
        analyzeCompositionMidiCorpus(
            record);

    expect(
        analysis.notesPerBeat == 1.5,
        "notes-per-beat is calculated correctly");

    expect(
        analysis.averagePitch ==
            (60.0 + 64.0 + 67.0) /
            3.0,
        "average pitch is calculated correctly");

    expect(
        analysis.pitchRange == 7.0,
        "pitch range is calculated correctly");

    expect(
        analysis.averageVelocity ==
            (100.0 + 80.0 + 90.0) /
            3.0,
        "average velocity is calculated correctly");

    expect(
        analysis.averageDurationBeats == 1.0,
        "average duration is calculated correctly");

    expect(
        analysis.maxPolyphony == 2.0,
        "maximum polyphony is calculated correctly");
}

void testDeterministicAnalysis()
{
    const auto record =
        buildRecord();

    const auto first =
        analyzeCompositionMidiCorpus(
            record);

    const auto second =
        analyzeCompositionMidiCorpus(
            record);

    expect(
        first.notesPerBeat ==
            second.notesPerBeat &&
        first.averagePitch ==
            second.averagePitch &&
        first.pitchRange ==
            second.pitchRange &&
        first.averageVelocity ==
            second.averageVelocity &&
        first.averageDurationBeats ==
            second.averageDurationBeats &&
        first.maxPolyphony ==
            second.maxPolyphony,
        "MIDI corpus analysis is deterministic");
}

void testInvalidRecordIsRejected()
{
    CompositionMidiCorpusRecord invalid;

    const auto analysis =
        analyzeCompositionMidiCorpus(
            invalid);

    expect(
        !analysis.isValid(),
        "invalid MIDI corpus record is rejected");
}

void testZeroLengthIsRejected()
{
    auto record =
        buildRecord();

    record.lengthTicks =
        0;

    const auto analysis =
        analyzeCompositionMidiCorpus(
            record);

    expect(
        !analysis.isValid(),
        "zero-length MIDI record is rejected");
}

void testOverlappingNotesDrivePolyphony()
{
    auto record =
        buildRecord();

    record.notes.push_back(
        CompositionMidiNote{
            0, 72, 100,
            240, 720});

    const auto analysis =
        analyzeCompositionMidiCorpus(
            record);

    expect(
        analysis.isValid(),
        "overlapping-note analysis remains valid");

    expect(
        analysis.maxPolyphony == 3.0,
        "overlapping notes increase measured polyphony");
}

} // namespace

int main()
{
    testAnalysisIsValid();
    testBasicMetrics();
    testDeterministicAnalysis();
    testInvalidRecordIsRejected();
    testZeroLengthIsRejected();
    testOverlappingNotesDrivePolyphony();

    std::cout
        << "MIDI-GenGX MIDI corpus analysis tests passed.\n";

    return 0;
}
