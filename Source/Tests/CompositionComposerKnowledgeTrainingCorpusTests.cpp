#include "Music/CompositionComposerKnowledgeTrainingCorpus.h"

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

CompositionComposerKnowledgeSample makeSample(
    const std::string& id,
    const std::string& composer,
    const std::string& work)
{
    CompositionDatasetSample composition;
    composition.sampleId = id;
    composition.globalFeatures = {0.25, 0.50, 0.25};
    composition.sectionFeatures =
    {
        {0.0, 0.2, 0.0, 0.25, 0.0, 0.0}
    };
    composition.analysisValid = true;

    CompositionSequenceMetadata metadata;
    metadata.sampleId = id;
    metadata.composerId = composer;
    metadata.workId = work;
    metadata.movementId = id;
    metadata.styleId = "style";
    metadata.eraId = "era";
    metadata.instrumentationId = "solo_piano";
    metadata.verified = true;
    metadata.valid = true;

    return buildCompositionComposerKnowledgeSample(composition, metadata);
}

CompositionComposerKnowledgeCatalog makeCatalog()
{
    return buildCompositionComposerKnowledgeCatalog(
    {
        makeSample("bach-w1", "bach", "w1"),
        makeSample("bach-w2", "bach", "w2"),
        makeSample("bach-w3", "bach", "w3"),
        makeSample("chopin-w1", "chopin", "w1"),
        makeSample("chopin-w2", "chopin", "w2"),
        makeSample("chopin-w3", "chopin", "w3"),
        makeSample("chopin-w4", "chopin", "w4")
    });
}

void testBuildProducesReadyCorpus()
{
    const auto catalog = makeCatalog();
    const auto partition =
        buildCompositionComposerKnowledgePartition(catalog, 0.25, 0.25);
    const auto manifest =
        buildCompositionComposerKnowledgeCorpusManifest(
            catalog, partition, "classical-composers", "2026.08.24");

    const auto corpus =
        buildCompositionComposerKnowledgeTrainingCorpus(
            catalog, partition, manifest);

    expect(corpus.isValid(), "training corpus is valid");
    expect(corpus.sampleCount() == catalog.sampleCount(),
           "all catalog samples are preserved");
    expect(corpus.trainingCount() == manifest.trainingCount &&
           corpus.validationCount() == manifest.validationCount &&
           corpus.testCount() == manifest.testCount,
           "split counts match the manifest");
}

void testSampleIdentityIsPreserved()
{
    const auto catalog = makeCatalog();
    const auto partition =
        buildCompositionComposerKnowledgePartition(catalog, 0.25, 0.25);
    const auto manifest =
        buildCompositionComposerKnowledgeCorpusManifest(
            catalog, partition, "classical-composers", "2026.08.24");

    const auto corpus =
        buildCompositionComposerKnowledgeTrainingCorpus(
            catalog, partition, manifest);

    expect(corpus.isValid(), "identity corpus is valid");

    for (const auto* split :
         {
             &corpus.trainingSamples,
             &corpus.validationSamples,
             &corpus.testSamples
         })
    {
        for (const auto& sample : *split)
        {
            expect(
                sample.composition.sampleId ==
                    sample.metadata.sampleId,
                "composition and metadata sample IDs remain aligned");
        }
    }
}

void testManifestMismatchFailsClosed()
{
    const auto catalog = makeCatalog();
    const auto partition =
        buildCompositionComposerKnowledgePartition(catalog, 0.25, 0.25);
    auto manifest =
        buildCompositionComposerKnowledgeCorpusManifest(
            catalog, partition, "classical-composers", "2026.08.24");

    ++manifest.trainingCount;

    const auto corpus =
        buildCompositionComposerKnowledgeTrainingCorpus(
            catalog, partition, manifest);

    expect(!corpus.isValid(), "manifest mismatch is rejected");
}

void testInvalidPartitionFailsClosed()
{
    const auto catalog = makeCatalog();
    CompositionComposerKnowledgePartition invalid;
    const auto validPartition =
        buildCompositionComposerKnowledgePartition(catalog, 0.25, 0.25);
    const auto manifest =
        buildCompositionComposerKnowledgeCorpusManifest(
            catalog, validPartition, "classical-composers", "2026.08.24");

    const auto corpus =
        buildCompositionComposerKnowledgeTrainingCorpus(
            catalog, invalid, manifest);

    expect(!corpus.isValid(), "invalid partition is rejected");
}

} // namespace

int main()
{
    testBuildProducesReadyCorpus();
    testSampleIdentityIsPreserved();
    testManifestMismatchFailsClosed();
    testInvalidPartitionFailsClosed();

    std::cout
        << "MIDI-GenGX Phase 100 composer knowledge training corpus tests passed.\n";
    return 0;
}
