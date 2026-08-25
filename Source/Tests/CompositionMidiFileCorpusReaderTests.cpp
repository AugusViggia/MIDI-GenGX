#include "Music/CompositionMidiFileCorpusReader.h"

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

void appendU32BE(
    std::vector<std::uint8_t>& bytes,
    std::uint32_t value)
{
    bytes.push_back(
        static_cast<std::uint8_t>(
            value >> 24));
    bytes.push_back(
        static_cast<std::uint8_t>(
            value >> 16));
    bytes.push_back(
        static_cast<std::uint8_t>(
            value >> 8));
    bytes.push_back(
        static_cast<std::uint8_t>(
            value));
}

std::vector<std::uint8_t> buildSimpleMidi()
{
    std::vector<std::uint8_t> bytes =
    {
        'M','T','h','d'
    };

    appendU32BE(
        bytes,
        6);

    bytes.push_back(0);
    bytes.push_back(0); // format 0

    bytes.push_back(0);
    bytes.push_back(1); // one track

    bytes.push_back(0x01);
    bytes.push_back(0xE0); // 480 PPQ

    const std::vector<std::uint8_t> track =
    {
        0x00, 0x90, 60, 100,
        0x83, 0x60, 0x80, 60, 0,
        0x00, 0xFF, 0x2F, 0x00
    };

    bytes.push_back('M');
    bytes.push_back('T');
    bytes.push_back('r');
    bytes.push_back('k');

    appendU32BE(
        bytes,
        static_cast<std::uint32_t>(
            track.size()));

    bytes.insert(
        bytes.end(),
        track.begin(),
        track.end());

    return bytes;
}

void testReadsSimpleMidi()
{
    const auto midi =
        buildSimpleMidi();

    CompositionMidiFileCorpusReader reader;

    const auto record =
        reader.read(
            "simple",
            midi.data(),
            midi.size());

    expect(
        record.isValid(),
        "simple MIDI corpus record is valid");

    expect(
        record.ticksPerQuarterNote == 480,
        "MIDI division is preserved");

    expect(
        record.trackCount == 1,
        "track count is preserved");

    expect(
        record.noteCount() == 1,
        "one note is extracted");

    expect(
        record.notes.front().midiNote == 60 &&
        record.notes.front().velocity == 100 &&
        record.notes.front().startTick == 0 &&
        record.notes.front().endTick == 480,
        "note timing and pitch are extracted");

    expect(
        record.lengthTicks == 480,
        "MIDI length is extracted");
}

void testDeterministicRead()
{
    const auto midi =
        buildSimpleMidi();

    CompositionMidiFileCorpusReader reader;

    const auto first =
        reader.read(
            "simple",
            midi.data(),
            midi.size());

    const auto second =
        reader.read(
            "simple",
            midi.data(),
            midi.size());

    expect(
        first.isValid() &&
        second.isValid(),
        "repeated MIDI reads remain valid");

    expect(
        first.notes.size() ==
            second.notes.size() &&
        first.lengthTicks ==
            second.lengthTicks,
        "repeated MIDI reads are deterministic");
}

void testInvalidHeaderIsRejected()
{
    const std::vector<std::uint8_t> invalid =
    {
        'N','O','P','E'
    };

    CompositionMidiFileCorpusReader reader;

    const auto record =
        reader.read(
            "invalid",
            invalid.data(),
            invalid.size());

    expect(
        !record.isValid(),
        "invalid MIDI header is rejected");
}

void testNullInputIsRejected()
{
    CompositionMidiFileCorpusReader reader;

    const auto record =
        reader.read(
            "null",
            nullptr,
            0);

    expect(
        !record.isValid(),
        "null MIDI input is rejected");
}

void testTruncatedTrackIsRejected()
{
    auto midi =
        buildSimpleMidi();

    midi.pop_back();

    CompositionMidiFileCorpusReader reader;

    const auto record =
        reader.read(
            "truncated",
            midi.data(),
            midi.size());

    expect(
        !record.isValid(),
        "truncated MIDI track is rejected");
}

void testInvalidSampleIdIsRejected()
{
    const auto midi =
        buildSimpleMidi();

    CompositionMidiFileCorpusReader reader;

    const auto record =
        reader.read(
            {},
            midi.data(),
            midi.size());

    expect(
        !record.isValid(),
        "empty MIDI sample id is rejected");
}

} // namespace

int main()
{
    testReadsSimpleMidi();
    testDeterministicRead();
    testInvalidHeaderIsRejected();
    testNullInputIsRejected();
    testTruncatedTrackIsRejected();
    testInvalidSampleIdIsRejected();

    std::cout
        << "MIDI-GenGX MIDI corpus reader tests passed.\n";

    return 0;
}
