#include "Music/MotifRecurrenceProfile.h"
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

void testFamiliesGroupCanonicalIdentity()
{
    const auto base =
        makeMotif();

    const auto transposed =
        MotifDevelopment::transpose(
            base,
            7);

    const auto graph =
        buildMotifOccurrenceGraph(
            {base, transposed, base},
            {0, 4, 8});

    const auto profile =
        analyzeMotifRecurrence(graph);

    expect(
        profile.isValid(),
        "recurrence profile is valid");

    expect(
        profile.families.size() == 1,
        "transposed identity belongs to one recurrence family");

    const auto& family =
        profile.families.front();

    expect(
        family.occurrenceIndices.size() == 3,
        "family contains all repeated occurrences");

    expect(
        family.isRecurring(),
        "family is marked recurring");

    expect(
        family.firstPhraseIndex == 0 &&
        family.lastPhraseIndex == 8,
        "family tracks recurrence span");
}

void testTransformationCounts()
{
    const auto base =
        makeMotif();

    const auto transposed =
        MotifDevelopment::transpose(base, 5);

    const auto retrograde =
        MotifDevelopment::retrograde(base);

    const auto inversion =
        MotifDevelopment::invert(base);

    auto rhythmic =
        base;
    rhythmic.notes[1].durationBeats = 0.75;

    const auto graph =
        buildMotifOccurrenceGraph(
            {
                base,
                transposed,
                retrograde,
                inversion,
                rhythmic
            },
            {0, 2, 4, 6, 8});

    const auto profile =
        analyzeMotifRecurrence(graph);

    const auto family =
        profile.findFamily(
            fingerprintMotif(base));

    expect(
        family != nullptr,
        "base motif family can be found");

    expect(
        family->transpositionCount >= 1,
        "family records transposition relationships");

    expect(
        family->retrogradeCount >= 1,
        "family records retrograde relationships");

    expect(
        family->inversionCount >= 1,
        "family records inversion relationships");

    expect(
        family->rhythmicVariationCount >= 1,
        "family records rhythmic variation");

    expect(
        family->transformationCount() >= 4,
        "family totals transformation relationships");
}

void testUnrelatedMotifsRemainSeparate()
{
    auto different =
        makeMotif();

    different.notes[2].relativePitch =
        11;

    const auto graph =
        buildMotifOccurrenceGraph(
            {makeMotif(), different},
            {1, 9});

    const auto profile =
        analyzeMotifRecurrence(graph);

    expect(
        profile.families.size() == 2,
        "unrelated motifs remain separate families");

    expect(
        profile.findFamily(
            fingerprintMotif(makeMotif())) != nullptr,
        "original family remains searchable");
}

void testInvalidGraphProducesEmptyProfile()
{
    MotifOccurrenceGraph invalid;

    const auto profile =
        analyzeMotifRecurrence(invalid);

    expect(
        !profile.analysisValid,
        "invalid graph is marked as unanalyzed");

    expect(
        !profile.isValid(),
        "invalid graph does not become a falsely valid profile");
}

void testValidEmptyGraphProducesValidEmptyProfile()
{
    const auto graph =
        buildMotifOccurrenceGraph({});

    const auto profile =
        analyzeMotifRecurrence(graph);

    expect(
        profile.analysisValid,
        "valid empty graph completes analysis");

    expect(
        profile.isValid(),
        "valid empty graph produces a valid empty profile");

    expect(
        profile.families.empty(),
        "valid empty graph has no motif families");
}

} // namespace

int main()
{
    testFamiliesGroupCanonicalIdentity();
    testTransformationCounts();
    testUnrelatedMotifsRemainSeparate();
    testInvalidGraphProducesEmptyProfile();
    testValidEmptyGraphProducesValidEmptyProfile();

    std::cout
        << "MIDI-GenGX Motif Recurrence Profile tests passed.\n";

    return 0;
}
