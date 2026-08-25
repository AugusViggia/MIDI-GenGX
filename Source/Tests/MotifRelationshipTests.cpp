#include "Music/MotifRelationship.h"
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
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}

Motif makeMotif()
{
    Motif motif;
    motif.lengthBeats = 2.0;
    motif.notes =
    {
        MotifNote{0.0, 0.5, 0, 0},
        MotifNote{0.5, 0.5, 2, 1},
        MotifNote{1.0, 0.5, 5, -1},
        MotifNote{1.5, 0.5, 3, 0}
    };
    return motif;
}

void testIdentity()
{
    const auto motif = makeMotif();
    const auto relationship = analyzeMotifRelationship(motif, motif);

    expect(relationship.isValid(), "identity relationship is valid");
    expect(relationship.kind == MotifRelationshipKind::Identity,
           "equal motifs are classified as identity");
    expect(relationship.confidence == 1.0,
           "identity has full confidence");
}

void testTransposition()
{
    const auto source = makeMotif();
    const auto candidate =
        MotifDevelopment::transpose(
            source,
            7);

    const auto sourceFingerprint =
        fingerprintMotif(source);

    const auto candidateFingerprint =
        fingerprintMotif(candidate);

    expect(
        sourceFingerprint.canonicalKey() ==
            candidateFingerprint.canonicalKey(),
        "canonical fingerprint remains transposition-invariant");

    const auto relationship =
        analyzeMotifRelationship(
            source,
            candidate);

    expect(
        relationship.kind ==
            MotifRelationshipKind::Transposition,
        "transposed motif is classified as transposition");

    expect(
        relationship.transpositionSemitones == 7,
        "transposition amount is recovered");

    expect(
        relationship.confidence == 1.0,
        "exact transposition has full confidence");
}

void testRetrogradeAndInversion()
{
    const auto source = makeMotif();
    const auto retrograde = MotifDevelopment::retrograde(source);
    const auto inversion = MotifDevelopment::invert(source);

    expect(
        analyzeMotifRelationship(source, retrograde).kind ==
            MotifRelationshipKind::Retrograde,
        "retrograde is recognized");

    expect(
        analyzeMotifRelationship(source, inversion).kind ==
            MotifRelationshipKind::Inversion,
        "inversion is recognized");
}

void testRhythmicVariation()
{
    const auto source = makeMotif();
    auto candidate = source;
    candidate.notes[1].durationBeats = 0.75;

    const auto relationship =
        analyzeMotifRelationship(source, candidate);

    expect(
        relationship.kind ==
            MotifRelationshipKind::RhythmicVariation,
        "rhythmic variation is recognized");

    expect(
        relationship.confidence > 0.9,
        "rhythmic variation confidence is high");
}

void testIntervalVariation()
{
    const auto source = makeMotif();
    auto candidate = source;
    candidate.notes[2].relativePitch += 2;

    const auto relationship =
        analyzeMotifRelationship(source, candidate);

    expect(
        relationship.kind ==
            MotifRelationshipKind::IntervalVariation,
        "interval variation is recognized");
}

void testCompoundVariation()
{
    const auto source = makeMotif();
    auto candidate = source;
    candidate.notes[1].durationBeats = 0.75;
    candidate.notes[2].relativePitch += 2;

    const auto relationship =
        analyzeMotifRelationship(source, candidate);

    expect(
        relationship.kind ==
            MotifRelationshipKind::CompoundVariation,
        "compound variation is recognized");

    expect(
        relationship.confidence > 0.0 &&
        relationship.confidence < 1.0,
        "compound variation confidence is partial");
}

void testInvalidInput()
{
    Motif invalid;
    const auto relationship =
        analyzeMotifRelationship(invalid, makeMotif());

    expect(
        !relationship.isValid(),
        "invalid motif relationship is rejected");
}

} // namespace

int main()
{
    testIdentity();
    testTransposition();
    testRetrogradeAndInversion();
    testRhythmicVariation();
    testIntervalVariation();
    testCompoundVariation();
    testInvalidInput();

    std::cout
        << "MIDI-GenGX Motif Relationship tests passed.\n";

    return 0;
}
