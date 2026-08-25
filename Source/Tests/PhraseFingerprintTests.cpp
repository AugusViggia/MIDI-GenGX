#include "Music/PhraseFingerprint.h"

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

Phrase makePhrase(
    std::initializer_list<int> pitches)
{
    Phrase phrase;
    phrase.lengthBeats = 4.0;

    double beat = 0.0;

    for (const int pitch : pitches)
    {
        phrase.notes.push_back(
            NoteEvent{
                pitch,
                100,
                beat,
                0.5,
                1});

        beat += 0.5;
    }

    return phrase;
}

void testAscendingFingerprint()
{
    const auto phrase =
        makePhrase({60, 62, 64, 67});

    const auto fp =
        fingerprintPhrase(
            phrase,
            {0, 2, 4, 5, 7, 9, 11});

    expect(
        fp.noteCount == 4,
        "fingerprint counts notes");

    expect(
        fp.lowestMidi == 60 &&
        fp.highestMidi == 67,
        "fingerprint tracks register extremes");

    expect(
        fp.stepRatio > 0.0 &&
        fp.leapRatio > 0.0,
        "fingerprint separates steps and leaps");

    expect(
        fp.upwardMotionRatio == 1.0,
        "ascending phrase has only upward motion");

    expect(
        fp.melodicProfile ==
            MelodicProfile::Ascending,
        "ascending phrase gets ascending profile");

    expect(
        fp.isScaleContained,
        "scale-contained phrase is identified");
}

void testRepeatedRhythmAndPitch()
{
    Phrase phrase;
    phrase.lengthBeats = 4.0;
    phrase.notes =
    {
        NoteEvent{60, 100, 0.0, 1.0, 1},
        NoteEvent{60, 100, 1.0, 1.0, 1},
        NoteEvent{60, 100, 2.0, 1.0, 1}
    };

    const auto fp =
        fingerprintPhrase(
            phrase,
            {0, 2, 4, 5, 7, 9, 11});

    expect(
        fp.repeatedPitchRatio == 1.0,
        "repeated pitch ratio reaches one");

    expect(
        fp.stationaryMotionRatio == 1.0,
        "repeated pitch has stationary motion");

    expect(
        fp.upwardMotionRatio == 0.0 &&
        fp.downwardMotionRatio == 0.0,
        "stationary phrase has no directional motion");

    expect(
        fp.melodicProfile ==
            MelodicProfile::Stationary,
        "stationary phrase gets stationary profile");
}

void testScaleFailureAndPolyphony()
{
    Phrase phrase;
    phrase.lengthBeats = 4.0;
    phrase.notes =
    {
        NoteEvent{60, 100, 0.0, 2.0, 1},
        NoteEvent{61, 100, 1.0, 1.0, 1},
        NoteEvent{67, 100, 2.0, 1.0, 1}
    };

    const auto fp =
        fingerprintPhrase(
            phrase,
            {0, 2, 4, 5, 7, 9, 11});

    expect(
        !fp.isMonophonic,
        "overlapping notes are detected as polyphonic");

    expect(
        !fp.isScaleContained,
        "out-of-scale pitch is detected");

    expect(
        fp.noteDensityPerBeat ==
            0.75,
        "note density uses phrase duration");
}

void testRegisterNormalization()
{
    Phrase phrase =
        makePhrase({48, 60, 72});

    const auto fp =
        fingerprintPhrase(
            phrase);

    expect(
        fp.averageMidi == 60.0,
        "fingerprint calculates average pitch");

    expect(
        fp.registerPosition == 0.5,
        "register position is normalized");
}

} // namespace

int main()
{
    testAscendingFingerprint();
    testRepeatedRhythmAndPitch();
    testScaleFailureAndPolyphony();
    testRegisterNormalization();

    std::cout
        << "MIDI-GenGX Phrase Fingerprint tests passed.\n";

    return 0;
}
