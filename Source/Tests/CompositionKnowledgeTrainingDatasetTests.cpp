#include "Music/CompositionKnowledgeTrainingDatasetArtifact.h"
#include "Music/CompositionConditioningVocabulary.h"

#include <cstdlib>
#include <iostream>

using namespace midigengx::music;
using midigengx::domain::GenrePreset;
using midigengx::domain::SoundEngineeringIntent;

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

CompositionMidiTrainingSequence makeSequence(const std::string& id)
{
    CompositionMidiTrainingSequence sequence;
    sequence.sampleId = id;
    sequence.analysisValid = true;

    CompositionMidiTrainingEvent event;
    event.features.resize(
        CompositionMidiTrainingEvent::featureCount,
        0.25);
    sequence.events.push_back(event);
    return sequence;
}

CompositionSequenceMetadata makeMetadata(
    const std::string& id,
    const std::string& composer,
    const std::string& style)
{
    CompositionSequenceMetadata metadata;
    metadata.sampleId = id;
    metadata.composerId = composer;
    metadata.workId = id + "_work";
    metadata.movementId = "movement_1";
    metadata.styleId = style;
    metadata.eraId = "romantic";
    metadata.instrumentationId = "solo_piano";
    metadata.verified = true;
    metadata.valid = true;
    return metadata;
}

CompositionComposerKnowledgeSampleRepresentation makeComposerKnowledge(
    const std::string& id,
    const std::string& composer)
{
    CompositionComposerKnowledgeSampleRepresentation result;
    result.sampleId = id;
    result.composerId = composer;
    result.features.fill(0.25);
    result.valid = true;
    return result;
}

CompositionSoundEngineeringKnowledgeRepresentation makeSoundEngineeringKnowledge()
{
    SoundEngineeringIntent intent;
    intent.lowEndOrganization = 80;
    intent.registerSeparation = 70;
    intent.rhythmicSpace = 60;
    intent.densityBudget = 50;
    intent.energyControl = 55;
    intent.grooveFocus = 65;

    return buildCompositionSoundEngineeringKnowledgeRepresentation(intent);
}

CompositionKnowledgeTrainingConditioning makeConditioning(
    const std::string& id,
    const std::string& composer,
    GenrePreset genre)
{
    CompositionKnowledgeTrainingConditioning conditioning;
    conditioning.sampleId = id;
    conditioning.composerKnowledge =
        makeComposerKnowledge(id, composer);
    conditioning.genreKnowledge =
        buildCompositionGenreKnowledgeRepresentation(genre);
    conditioning.soundEngineeringKnowledge =
        makeSoundEngineeringKnowledge();
    conditioning.valid = true;
    return conditioning;
}

CompositionConditionedTrainingDataset makeConditionedDataset()
{
    const auto metadata =
        buildCompositionSequenceMetadataCatalog(
        {
            makeMetadata("piece-b", "bach", "baroque"),
            makeMetadata("piece-a", "chopin", "romantic")
        });

    return buildCompositionConditionedTrainingDataset(
        {
            makeSequence("piece-b"),
            makeSequence("piece-a")
        },
        metadata);
}

void testKnowledgeDatasetJoinAndFlattening()
{
    const auto base = makeConditionedDataset();

    const auto dataset =
        buildCompositionKnowledgeTrainingDataset(
            base,
            {
                makeConditioning("piece-b", "bach", GenrePreset::ProgressiveHouse),
                makeConditioning("piece-a", "chopin", GenrePreset::ProgressiveHouse)
            });

    expect(
        dataset.isValid(),
        "knowledge dataset joins all required conditioning sources");

    expect(
        dataset.sampleCount() == 2 &&
        dataset.featureWidth == 37,
        "knowledge dataset exposes the fixed 37-feature conditioning width");

    expect(
        dataset.samples[0].sampleId == "piece-a" &&
        dataset.samples[1].sampleId == "piece-b",
        "knowledge dataset ordering is deterministic");

    expect(
        dataset.samples[0].conditioningFeatures[0] == 0.25 &&
        dataset.samples[0].conditioningFeatures[14] ==
            dataset.samples[0].genreKnowledge.features[0] &&
        dataset.samples[0].conditioningFeatures[29] ==
            dataset.samples[0].soundEngineeringKnowledge.features[0],
        "conditioning feature vector preserves source representation ordering");
}

void testMissingConditioningFailsClosed()
{
    const auto dataset =
        buildCompositionKnowledgeTrainingDataset(
            makeConditionedDataset(),
            {
                makeConditioning("piece-a", "chopin", GenrePreset::ProgressiveHouse)
            });

    expect(
        !dataset.isValid(),
        "missing per-sample conditioning prevents dataset construction");
}

void testDuplicateConditioningFailsClosed()
{
    const auto dataset =
        buildCompositionKnowledgeTrainingDataset(
            makeConditionedDataset(),
            {
                makeConditioning("piece-a", "chopin", GenrePreset::ProgressiveHouse),
                makeConditioning("piece-a", "chopin", GenrePreset::ProgressiveHouse),
                makeConditioning("piece-b", "bach", GenrePreset::ProgressiveHouse)
            });

    expect(
        !dataset.isValid(),
        "duplicate sample conditioning is rejected");
}

void testArtifactIsDeterministic()
{
    const auto dataset =
        buildCompositionKnowledgeTrainingDataset(
            makeConditionedDataset(),
            {
                makeConditioning("piece-b", "bach", GenrePreset::ProgressiveHouse),
                makeConditioning("piece-a", "chopin", GenrePreset::ProgressiveHouse)
            });

    const auto first =
        serializeCompositionKnowledgeTrainingDataset(dataset);
    const auto second =
        serializeCompositionKnowledgeTrainingDataset(dataset);

    expect(
        first.isValid() && second.isValid(),
        "knowledge training dataset artifact is structurally valid");

    expect(
        first.bytes == second.bytes,
        "knowledge training dataset artifact is deterministic");
}

} // namespace

int main()
{
    testKnowledgeDatasetJoinAndFlattening();
    testMissingConditioningFailsClosed();
    testDuplicateConditioningFailsClosed();
    testArtifactIsDeterministic();

    std::cout
        << "MIDI-GenGX Phase 118 knowledge training dataset tests passed.\n";

    return 0;
}
