#include "Music/CompositionLearningContract.h"
#include "Music/MotifDevelopment.h"

#include <cstdlib>
#include <iostream>
#include <string>
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

CompositionDatasetPreparedView buildPrepared(
    std::size_t sampleCount)
{
    const auto snapshot =
        buildSnapshot();

    std::vector<CompositionDatasetInput>
        inputs;

    for (std::size_t index = 0;
         index < sampleCount;
         ++index)
    {
        inputs.push_back(
        {
            "contract-" +
                std::to_string(index),
            snapshot
        });
    }

    const auto dataset =
        buildCompositionDataset(
            inputs);

    const auto quality =
        assessCompositionDatasetQuality(
            dataset);

    const auto partition =
        buildCompositionDatasetPartition(
            dataset,
            0.20,
            0.20);

    const auto manifest =
        buildCompositionDatasetManifest(
            dataset,
            quality,
            partition);

    return prepareCompositionDatasetForLearning(
        dataset,
        quality,
        manifest,
        partition);
}

void testNextSectionContract()
{
    const auto prepared =
        buildPrepared(20);

    const auto contract =
        buildCompositionLearningContract(
            prepared,
            LearningObjective::NextSectionPrediction);

    expect(
        contract.isValid(),
        "next-section learning contract is valid");

    expect(
        CompositionLearningContract::version == 1,
        "learning contract version is stable");

    expect(
        contract.objective ==
            LearningObjective::NextSectionPrediction,
        "next-section objective is preserved");

    expect(
        contract.globalInputWidth == 13 &&
        contract.sectionInputWidth == 6 &&
        contract.targetWidth == 6,
        "learning widths match dataset schema");

    expect(
        contract.contextLength ==
            prepared.normalizedBatch.maxSectionCount,
        "context length matches prepared batch");

    expect(
        contract.usesSectionMask,
        "next-section contract uses section mask");
}

void testReconstructionContract()
{
    const auto prepared =
        buildPrepared(5);

    const auto contract =
        buildCompositionLearningContract(
            prepared,
            LearningObjective::SectionStateReconstruction);

    expect(
        contract.isValid(),
        "reconstruction learning contract is valid");

    expect(
        contract.targetWidth == 6,
        "reconstruction target width matches section features");
}

void testContractIsDeterministic()
{
    const auto prepared =
        buildPrepared(20);

    const auto first =
        buildCompositionLearningContract(
            prepared,
            LearningObjective::NextSectionPrediction);

    const auto second =
        buildCompositionLearningContract(
            prepared,
            LearningObjective::NextSectionPrediction);

    expect(
        first.globalInputWidth ==
            second.globalInputWidth &&
        first.sectionInputWidth ==
            second.sectionInputWidth &&
        first.targetWidth ==
            second.targetWidth &&
        first.contextLength ==
            second.contextLength &&
        first.usesSectionMask ==
            second.usesSectionMask,
        "learning contract is deterministic");
}

void testInvalidPreparedViewIsRejected()
{
    CompositionDatasetPreparedView invalid;

    const auto contract =
        buildCompositionLearningContract(
            invalid,
            LearningObjective::NextSectionPrediction);

    expect(
        !contract.analysisValid,
        "invalid prepared view prevents contract creation");

    expect(
        !contract.isValid(),
        "invalid prepared view produces invalid contract");
}

void testWidthsCannotDriftFromSchema()
{
    auto prepared =
        buildPrepared(20);

    prepared.normalizedBatch.globalFeatureWidth =
        12;

    const auto contract =
        buildCompositionLearningContract(
            prepared,
            LearningObjective::NextSectionPrediction);

    expect(
        !contract.analysisValid,
        "schema width drift invalidates learning contract");
}

void testMaskRequirementCannotBeRemoved()
{
    auto prepared =
        buildPrepared(20);

    prepared.normalizedBatch.sectionMask.clear();

    const auto contract =
        buildCompositionLearningContract(
            prepared,
            LearningObjective::NextSectionPrediction);

    expect(
        !contract.analysisValid,
        "missing section mask prevents learning contract");
}

} // namespace

int main()
{
    testNextSectionContract();
    testReconstructionContract();
    testContractIsDeterministic();
    testInvalidPreparedViewIsRejected();
    testWidthsCannotDriftFromSchema();
    testMaskRequirementCannotBeRemoved();

    std::cout
        << "MIDI-GenGX Composition Learning Contract tests passed.\n";

    return 0;
}
