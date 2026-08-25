#include "Music/CompositionDatasetSchema.h"
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

CompositionDatasetSample buildSample()
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

    const auto snapshot =
        buildCompositionKnowledgeSnapshot(
            composition,
            graph,
            transitions);

    return buildCompositionDatasetSample(
        snapshot,
        "schema-test");
}

void testSchemaConstants()
{
    expect(
        CompositionDatasetSchema::version == 1,
        "dataset schema version is one");

    expect(
        CompositionDatasetSchema::globalFeatureCount == 13,
        "global schema width is thirteen");

    expect(
        CompositionDatasetSchema::sectionFeatureCount == 6,
        "section schema width is six");
}

void testFeatureIndicesAreStable()
{
    expect(
        CompositionDatasetSchema::globalFeatureIndex(
            GlobalFeature::TotalLengthNormalized) == 0,
        "first global feature index is stable");

    expect(
        CompositionDatasetSchema::globalFeatureIndex(
            GlobalFeature::PeakTensionNormalized) == 12,
        "last global feature index is stable");

    expect(
        CompositionDatasetSchema::sectionFeatureIndex(
            SectionFeature::RoleEncoded) == 0,
        "first section feature index is stable");

    expect(
        CompositionDatasetSchema::sectionFeatureIndex(
            SectionFeature::HarmonicDegreeDeltaNormalized) == 5,
        "last section feature index is stable");
}

void testFeatureNamesAreStable()
{
    expect(
        std::string(
            CompositionDatasetSchema::globalFeatureName(
                GlobalFeature::TotalLengthNormalized)) ==
            "total_length",
        "global feature name is stable");

    expect(
        std::string(
            CompositionDatasetSchema::globalFeatureName(
                GlobalFeature::PeakTensionNormalized)) ==
            "peak_tension",
        "peak feature name is stable");

    expect(
        std::string(
            CompositionDatasetSchema::sectionFeatureName(
                SectionFeature::RoleEncoded)) ==
            "role",
        "section role name is stable");

    expect(
        std::string(
            CompositionDatasetSchema::sectionFeatureName(
                SectionFeature::HarmonicDegreeDeltaNormalized)) ==
            "harmonic_degree_delta",
        "section transition name is stable");
}

void testValidSampleMatchesSchema()
{
    const auto sample =
        buildSample();

    expect(
        CompositionDatasetSchema::validateSample(
            sample),
        "valid sample conforms to schema");
}

void testGlobalWidthDriftIsRejected()
{
    auto sample =
        buildSample();

    sample.globalFeatures.push_back(
        0.0);

    expect(
        !CompositionDatasetSchema::validateSample(
            sample),
        "global feature width drift is rejected");
}

void testSectionWidthDriftIsRejected()
{
    auto sample =
        buildSample();

    sample.sectionFeatures.front().push_back(
        0.0);

    expect(
        !CompositionDatasetSchema::validateSample(
            sample),
        "section feature width drift is rejected");
}

void testSchemaVersionContractIsStable()
{
    auto sample =
        buildSample();

    expect(
        sample.schemaVersion ==
            CompositionDatasetSchema::version,
        "sample schema version matches schema contract");

    expect(
        CompositionDatasetSchema::version ==
            1,
        "schema version contract remains version one");
}

} // namespace

int main()
{
    testSchemaConstants();
    testFeatureIndicesAreStable();
    testFeatureNamesAreStable();
    testValidSampleMatchesSchema();
    testGlobalWidthDriftIsRejected();
    testSectionWidthDriftIsRejected();
    testSchemaVersionContractIsStable();

    std::cout
        << "MIDI-GenGX Composition Dataset Schema tests passed.\n";

    return 0;
}
