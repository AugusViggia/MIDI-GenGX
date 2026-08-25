#include "Music/MotifFingerprint.h"
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
        std::cerr
            << "FAILED: "
            << message
            << '\n';
        std::exit(1);
    }
}

Motif makeMotif(
    int firstPitchOffset = 0)
{
    Motif motif;
    motif.lengthBeats = 2.0;
    motif.notes =
    {
        MotifNote{0.0, 0.5, firstPitchOffset + 0, 0},
        MotifNote{0.5, 0.5, firstPitchOffset + 2, 1},
        MotifNote{1.0, 0.5, firstPitchOffset + 5, -1},
        MotifNote{1.5, 0.5, firstPitchOffset + 3, 0}
    };
    return motif;
}

void testFingerprintCapturesIdentity()
{
    const auto motif =
        makeMotif();

    const auto fp =
        fingerprintMotif(motif);

    expect(
        fp.isValid(),
        "motif fingerprint is valid");

    expect(
        fp.noteCount == 4,
        "motif fingerprint counts notes");

    expect(
        fp.intervalSignature ==
            std::vector<int>({2, 3, -2}),
        "motif fingerprint captures interval signature");

    expect(
        fp.contourSignature ==
            std::vector<int>({1, 1, -1}),
        "motif fingerprint captures contour signature");

    expect(
        fp.rhythmSignature ==
            std::vector<int>({4, 4, 4, 4}),
        "motif fingerprint quantizes rhythm signature");

    expect(
        !fp.canonicalKey().empty(),
        "motif fingerprint creates canonical key");
}

void testTranspositionPreservesIdentity()
{
    const auto original =
        makeMotif(0);

    const auto transposed =
        MotifDevelopment::transpose(
            original,
            7);

    const auto a =
        fingerprintMotif(original);

    const auto b =
        fingerprintMotif(transposed);

    expect(
        isMotivicIdentityEquivalent(
            a,
            b),
        "transposition preserves motif identity");

    expect(
        a.canonicalKey() ==
            b.canonicalKey(),
        "transposed motif has same canonical key");
}

void testRhythmicChangeBreaksIdentity()
{
    auto original =
        makeMotif();

    auto changed =
        original;

    changed.notes[2].durationBeats = 0.75;

    const auto a =
        fingerprintMotif(original);

    const auto b =
        fingerprintMotif(changed);

    expect(
        !isMotivicIdentityEquivalent(
            a,
            b),
        "rhythmic transformation changes motif identity");
}

void testIntervalChangeBreaksIdentity()
{
    auto original =
        makeMotif();

    auto changed =
        original;

    changed.notes[2].relativePitch += 2;

    const auto a =
        fingerprintMotif(original);

    const auto b =
        fingerprintMotif(changed);

    expect(
        !isMotivicIdentityEquivalent(
            a,
            b),
        "interval transformation changes motif identity");
}

void testInvalidMotifFingerprint()
{
    Motif invalid;

    const auto fp =
        fingerprintMotif(invalid);

    expect(
        !fp.isValid(),
        "invalid motif produces invalid fingerprint");

    expect(
        fp.canonicalKey().empty(),
        "invalid fingerprint has no canonical key");
}

} // namespace

int main()
{
    testFingerprintCapturesIdentity();
    testTranspositionPreservesIdentity();
    testRhythmicChangeBreaksIdentity();
    testIntervalChangeBreaksIdentity();
    testInvalidMotifFingerprint();

    std::cout
        << "MIDI-GenGX Motif Fingerprint tests passed.\n";

    return 0;
}
