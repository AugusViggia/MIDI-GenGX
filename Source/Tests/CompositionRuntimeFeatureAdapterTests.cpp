#include "Music/CompositionRuntimeFeatureAdapter.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

using namespace midigengx::music;
using namespace midigengx::domain;

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

MusicalContext buildContext()
{
    MusicalContext context;

    context.key = Key::G;
    context.scale = Scale{
        ScaleType::Minor};
    context.role = Role::Lead;

    context.parameters.lengthBars = 16;
    context.parameters.phraseLengthBars = 4;
    context.parameters.octaveLow = 0;
    context.parameters.octaveHigh = 2;
    context.parameters.tension = 70;
    context.parameters.complexity = 60;

    context.normalize();

    return context;
}

void testRuntimeFeatureShape()
{
    CompositionRuntimeFeatureAdapter adapter;

    const auto features =
        adapter.build(
            buildContext());

    expect(
        features.isValid(),
        "runtime features are valid");

    expect(
        features.globalFeatures.size() ==
            CompositionDatasetSchema::globalFeatureCount,
        "global runtime feature width matches schema");

    expect(
        features.sectionFeatures.size() ==
            CompositionDatasetSchema::sectionFeatureCount,
        "section runtime feature width matches schema");
}

void testRuntimeFeaturesAreNormalized()
{
    CompositionRuntimeFeatureAdapter adapter;

    const auto features =
        adapter.build(
            buildContext());

    expect(
        features.isValid(),
        "normalized runtime feature vector is valid");

    for (const auto value :
         features.globalFeatures)
    {
        expect(
            std::isfinite(value) &&
            value >= -1.0 &&
            value <= 1.0,
            "global runtime feature remains normalized");
    }

    for (const auto value :
         features.sectionFeatures)
    {
        expect(
            std::isfinite(value) &&
            value >= -1.0 &&
            value <= 1.0,
            "section runtime feature remains normalized");
    }
}

void testContextChangesPropagate()
{
    CompositionRuntimeFeatureAdapter adapter;

    auto low =
        buildContext();

    auto high =
        low;

    low.parameters.tension = 10;
    high.parameters.tension = 90;

    const auto lowFeatures =
        adapter.build(
            low);

    const auto highFeatures =
        adapter.build(
            high);

    expect(
        lowFeatures.isValid() &&
        highFeatures.isValid(),
        "changed contexts produce valid features");

    const auto tensionIndex =
        CompositionDatasetSchema::sectionFeatureIndex(
            SectionFeature::TensionNormalized);

    expect(
        lowFeatures.sectionFeatures[tensionIndex] <
            highFeatures.sectionFeatures[tensionIndex],
        "tension change propagates to runtime features");
}

void testUnavailableRuntimeKnowledgeStaysNeutral()
{
    CompositionRuntimeFeatureAdapter adapter;

    const auto features =
        adapter.build(
            buildContext());

    const auto motifIndex =
        CompositionDatasetSchema::globalFeatureIndex(
            GlobalFeature::MotifFamilyCountNormalized);

    const auto risingIndex =
        CompositionDatasetSchema::globalFeatureIndex(
            GlobalFeature::RisingTransitionsNormalized);

    const auto tensionDeltaIndex =
        CompositionDatasetSchema::sectionFeatureIndex(
            SectionFeature::TensionDeltaNormalized);

    expect(
        features.globalFeatures[motifIndex] == 0.0 &&
        features.globalFeatures[risingIndex] == 0.0 &&
        features.sectionFeatures[tensionDeltaIndex] == 0.0,
        "unavailable runtime information remains neutral");
}

void testExtremeContextRemainsSafe()
{
    CompositionRuntimeFeatureAdapter adapter;

    auto context =
        buildContext();

    context.parameters.lengthBars =
        1000;
    context.parameters.tension =
        -100;
    context.parameters.complexity =
        500;

    const auto features =
        adapter.build(
            context);

    expect(
        features.isValid(),
        "extreme runtime context is safely normalized");
}

void testAdapterIsDeterministic()
{
    CompositionRuntimeFeatureAdapter adapter;

    const auto context =
        buildContext();

    const auto first =
        adapter.build(
            context);

    const auto second =
        adapter.build(
            context);

    expect(
        first.globalFeatures ==
            second.globalFeatures &&
        first.sectionFeatures ==
            second.sectionFeatures,
        "runtime feature adaptation is deterministic");
}

} // namespace

int main()
{
    testRuntimeFeatureShape();
    testRuntimeFeaturesAreNormalized();
    testContextChangesPropagate();
    testUnavailableRuntimeKnowledgeStaysNeutral();
    testExtremeContextRemainsSafe();
    testAdapterIsDeterministic();

    std::cout
        << "MIDI-GenGX Composition Runtime Feature Adapter tests passed.\n";

    return 0;
}
