#include "Music/CompositionDatasetQuality.h"
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

CompositionDataset buildDataset()
{
    const auto snapshot =
        buildSnapshot();

    return buildCompositionDataset(
        {
            {"sample-a", snapshot},
            {"sample-b", snapshot},
            {"sample-c", snapshot}
        });
}

void testQualityOfValidDataset()
{
    const auto dataset =
        buildDataset();

    const auto quality =
        assessCompositionDatasetQuality(
            dataset);

    expect(
        quality.isValid(),
        "valid dataset quality assessment passes");

    expect(
        quality.sampleCount == 3,
        "quality assessment counts samples");

    expect(
        quality.invalidSampleCount == 0 &&
        quality.duplicateIdCount == 0,
        "valid dataset has no invalid or duplicate ids");
}

void testSchemaWidthsAreStable()
{
    const auto dataset =
        buildDataset();

    const auto quality =
        assessCompositionDatasetQuality(
            dataset);

    expect(
        quality.globalFeatureWidth == 13,
        "global feature width is stable");

    expect(
        quality.sectionFeatureWidth == 6,
        "section feature width is stable");

    expect(
        quality.minSectionCount ==
            quality.maxSectionCount &&
        quality.minSectionCount == 4,
        "section count is consistent");
}

void testEmptyDatasetQuality()
{
    const auto dataset =
        buildCompositionDataset({});

    const auto quality =
        assessCompositionDatasetQuality(
            dataset);

    expect(
        quality.isValid(),
        "empty dataset is a valid quality result");

    expect(
        quality.sampleCount == 0 &&
        quality.globalFeatureWidth == 0 &&
        quality.sectionFeatureWidth == 0,
        "empty dataset quality is zeroed");
}

void testInvalidDatasetIsRejected()
{
    CompositionDataset dataset;

    CompositionDatasetSample invalid;
    invalid.analysisValid = true;
    invalid.sampleId = "broken";

    dataset.samples.push_back(
        invalid);

    const auto quality =
        assessCompositionDatasetQuality(
            dataset);

    expect(
        !quality.isValid(),
        "invalid dataset quality is rejected");

    expect(
        quality.invalidSampleCount > 0,
        "invalid samples are counted");
}

void testFeatureWidthDriftIsRejected()
{
    auto dataset =
        buildDataset();

    dataset.samples[1]
        .globalFeatures
        .push_back(0.0);

    const auto quality =
        assessCompositionDatasetQuality(
            dataset);

    expect(
        !quality.isValid(),
        "global feature width drift is rejected");
}

void testSectionWidthDriftIsRejected()
{
    auto dataset =
        buildDataset();

    dataset.samples[2]
        .sectionFeatures
        .front()
        .push_back(0.0);

    const auto quality =
        assessCompositionDatasetQuality(
            dataset);

    expect(
        !quality.isValid(),
        "section feature width drift is rejected");
}

} // namespace

int main()
{
    testQualityOfValidDataset();
    testSchemaWidthsAreStable();
    testEmptyDatasetQuality();
    testInvalidDatasetIsRejected();
    testFeatureWidthDriftIsRejected();
    testSectionWidthDriftIsRejected();

    std::cout
        << "MIDI-GenGX Composition Dataset Quality tests passed.\n";

    return 0;
}
