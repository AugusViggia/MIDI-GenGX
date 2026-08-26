#include "Music/CompositionComposerKnowledgeRepresentation.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace midigengx::music;

namespace
{

void expect(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

CompositionComposerKnowledgeSample makeSample(
    const std::string& sampleId,
    const std::string& composerId,
    double majorRatio,
    double minorRatio,
    double openingRatio,
    double cadenceRatio,
    double averageTension)
{
    CompositionComposerKnowledgeSample sample;
    sample.metadata.sampleId = sampleId;
    sample.metadata.composerId = composerId;
    sample.metadata.workId = sampleId;
    sample.metadata.styleId = "romantic";
    sample.metadata.eraId = "romantic";
    sample.metadata.instrumentationId = "solo_piano";
    sample.metadata.verified = true;
    sample.metadata.valid = true;

    sample.composition.sampleId = sampleId;
    sample.composition.analysisValid = true;
    sample.composition.globalFeatures =
    {
        0.25, // total length
        0.03125, // section count
        0.0625, // harmony event count
        0.03125, // total motif family count
        0.015625, // recurring motif family count
        0.25, // average motif occurrences
        averageTension,
        0.10,
        0.90,
        0.50, // rising transitions / 64
        0.25, // falling transitions / 64
        0.25, // flat transitions / 64
        0.90 // peak tension
    };

    const double sectionTotal = 4.0;
    sample.composition.sectionFeatures =
    {
        {0.0, averageTension, 0.0, 0.0, majorRatio > 0.0 ? 0.0 : 0.2, 0.0},
        {1.0 / 3.0, averageTension, 0.0, 0.0, minorRatio > 0.0 ? 0.2 : 0.0, 0.0},
        {2.0 / 3.0, averageTension, 0.0, 0.0, 0.0, 0.0},
        {1.0, averageTension, 0.0, 0.0, 0.0, 0.0}
    };

    // Encode requested phrase ratios in a four-section deterministic sample.
    const double openingTarget = std::round(openingRatio * sectionTotal);
    const double cadenceTarget = std::round(cadenceRatio * sectionTotal);
    for (std::size_t i = 0; i < sample.composition.sectionFeatures.size(); ++i)
    {
        if (i < static_cast<std::size_t>(openingTarget))
            sample.composition.sectionFeatures[i][0] = 0.0;
        else if (i >= sample.composition.sectionFeatures.size() - static_cast<std::size_t>(cadenceTarget))
            sample.composition.sectionFeatures[i][0] = 1.0;
    }

    // Keep the remaining two section roles deterministic.
    if (sample.composition.sectionFeatures.size() >= 4)
    {
        sample.composition.sectionFeatures[1][0] = 1.0 / 3.0;
        sample.composition.sectionFeatures[2][0] = 2.0 / 3.0;
    }

    sample.valid = true;
    return sample;
}

void testSampleRepresentation()
{
    auto sample = makeSample(
        "chopin-a",
        "chopin",
        0.5,
        0.5,
        0.25,
        0.25,
        0.60);

    const auto representation =
        buildCompositionComposerKnowledgeSampleRepresentation(sample);

    expect(
        representation.isValid(),
        "valid composer sample should produce a valid representation");

    expect(
        representation.sampleId == "chopin-a" &&
        representation.composerId == "chopin",
        "representation should preserve sample and composer identity");

    for (const auto value : representation.features)
        expect(
            value >= 0.0 && value <= 1.0,
            "representation features must remain normalized");
}

void testComposerAggregationIsDeterministic()
{
    const auto first = makeSample(
        "chopin-a", "chopin", 0.5, 0.5, 0.25, 0.25, 0.40);
    const auto second = makeSample(
        "chopin-b", "chopin", 0.5, 0.5, 0.25, 0.25, 0.80);

    CompositionComposerKnowledgeGroup group;
    group.composerId = "chopin";
    group.samples = {first, second};

    const auto representation =
        buildCompositionComposerKnowledgeRepresentation(group);

    expect(
        representation.isValid(),
        "valid composer group should aggregate successfully");

    expect(
        representation.sampleCount == 2,
        "composer representation should preserve sample count");

    expect(
        std::abs(
            representation.features[CompositionComposerKnowledgeRepresentation::AverageSectionTension] -
            0.60) < 1.0e-9,
        "composer representation should average sample-level knowledge deterministically");
}

void testInvalidSampleFailsClosed()
{
    CompositionComposerKnowledgeSample sample;
    sample.metadata.composerId = "chopin";
    sample.metadata.sampleId = "invalid";

    const auto representation =
        buildCompositionComposerKnowledgeSampleRepresentation(sample);

    expect(
        !representation.isValid(),
        "invalid source sample must not produce a valid knowledge representation");
}

void testMixedComposerGroupFailsClosed()
{
    auto chopin = makeSample(
        "chopin-a", "chopin", 0.5, 0.5, 0.25, 0.25, 0.50);
    auto bach = makeSample(
        "bach-a", "bach", 0.5, 0.5, 0.25, 0.25, 0.50);

    CompositionComposerKnowledgeGroup group;
    group.composerId = "chopin";
    group.samples = {chopin, bach};

    const auto representation =
        buildCompositionComposerKnowledgeRepresentation(group);

    expect(
        !representation.isValid(),
        "mixed-composer knowledge group must fail closed");
}

} // namespace

int main()
{
    testSampleRepresentation();
    testComposerAggregationIsDeterministic();
    testInvalidSampleFailsClosed();
    testMixedComposerGroupFailsClosed();

    std::cout
        << "MIDI-GenGX Phase 116 composer knowledge representation tests passed.\n";

    return 0;
}
