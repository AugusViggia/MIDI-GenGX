#include "Music/CompositionDataset.h"
#include "Music/MotifDevelopment.h"

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

CompositionKnowledgeSnapshot buildSnapshot()
{
    midigengx::domain::MusicalContext context;

    context.parameters.lengthBars = 16;
    context.parameters.phraseLengthBars = 4;
    context.normalize();

    const auto structure =
        planPhraseStructure(context);

    const auto harmony =
        planHarmony(
            context,
            structure);

    const auto graph =
        buildCompositionKnowledgeGraph(
            structure,
            harmony);

    const auto transitions =
        analyzeCompositionTransitions(
            graph);

    const auto motifGraph =
        buildMotifOccurrenceGraph(
            {
                makeMotif(),
                MotifDevelopment::transpose(
                    makeMotif(),
                    7)
            },
            {0, 8});

    const auto motifProfile =
        analyzeMotifRecurrence(
            motifGraph);

    const auto catalog =
        buildMotifKnowledgeCatalog(
            motifProfile);

    const auto composition =
        buildCompositionKnowledgeRecord(
            structure,
            harmony,
            catalog);

    return buildCompositionKnowledgeSnapshot(
        composition,
        graph,
        transitions);
}

CompositionDatasetInput input(
    const std::string& id)
{
    return CompositionDatasetInput{
        id,
        buildSnapshot()
    };
}

void testDatasetValidityAndOrdering()
{
    const auto dataset =
        buildCompositionDataset(
            {
                input("composition-03"),
                input("composition-01"),
                input("composition-02")
            });

    expect(
        dataset.isValid(),
        "dataset is valid");

    expect(
        dataset.size() == 3,
        "dataset retains all valid samples");

    expect(
        dataset.samples[0].sampleId ==
            "composition-01" &&
        dataset.samples[1].sampleId ==
            "composition-02" &&
        dataset.samples[2].sampleId ==
            "composition-03",
        "dataset uses deterministic sample ordering");
}

void testStableLookup()
{
    const auto dataset =
        buildCompositionDataset(
            {
                input("alpha"),
                input("beta")
            });

    const auto* found =
        dataset.findById("beta");

    expect(
        found != nullptr,
        "dataset finds an existing sample");

    expect(
        found->sampleId == "beta",
        "dataset lookup returns requested sample");

    expect(
        dataset.findById("missing") == nullptr,
        "dataset rejects unknown sample id");

    expect(
        dataset.findById("") == nullptr,
        "dataset rejects empty sample id");
}

void testDuplicateIdsAreCollapsedDeterministically()
{
    const auto dataset =
        buildCompositionDataset(
            {
                input("same-id"),
                input("same-id"),
                input("other-id")
            });

    expect(
        dataset.isValid(),
        "dataset with duplicate input ids remains valid");

    expect(
        dataset.size() == 2,
        "duplicate sample ids collapse to one canonical record");

    expect(
        dataset.findById("same-id") != nullptr &&
        dataset.findById("other-id") != nullptr,
        "deduplicated samples remain searchable");
}

void testDatasetStatistics()
{
    const auto dataset =
        buildCompositionDataset(
            {
                input("one"),
                input("two")
            });

    expect(
        dataset.sectionCount() ==
            dataset.samples[0].sectionCount() +
            dataset.samples[1].sectionCount(),
        "dataset section count aggregates samples");

    expect(
        dataset.averageSectionCount() ==
            static_cast<double>(
                dataset.sectionCount()) /
            static_cast<double>(
                dataset.size()),
        "dataset average section count is correct");
}

void testInvalidSamplesAreExcluded()
{
    CompositionDatasetInput valid =
        input("valid");

    CompositionDatasetInput invalid;
    invalid.sampleId = "invalid";

    const auto dataset =
        buildCompositionDataset(
            {valid, invalid});

    expect(
        dataset.size() == 1,
        "invalid sample is excluded from dataset");

    expect(
        dataset.findById("valid") != nullptr,
        "valid sample remains available");
}

void testEmptyDatasetIsValid()
{
    const auto dataset =
        buildCompositionDataset({});

    expect(
        dataset.isValid(),
        "empty dataset is a valid container");

    expect(
        dataset.size() == 0 &&
        dataset.sectionCount() == 0 &&
        dataset.averageSectionCount() == 0.0,
        "empty dataset statistics are zero");

    expect(
        dataset.findById("anything") == nullptr,
        "empty dataset has no lookup result");
}

} // namespace

int main()
{
    testDatasetValidityAndOrdering();
    testStableLookup();
    testDuplicateIdsAreCollapsedDeterministically();
    testDatasetStatistics();
    testInvalidSamplesAreExcluded();
    testEmptyDatasetIsValid();

    std::cout
        << "MIDI-GenGX Composition Dataset tests passed.\n";

    return 0;
}
