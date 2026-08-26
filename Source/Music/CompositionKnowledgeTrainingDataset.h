#pragma once

#include "CompositionConditionedTrainingDataset.h"
#include "CompositionComposerKnowledgeRepresentation.h"
#include "CompositionGenreKnowledgeRepresentation.h"
#include "CompositionSoundEngineeringKnowledgeRepresentation.h"

#include <array>
#include <cstddef>
#include <string>
#include <vector>

namespace midigengx::music
{

struct CompositionKnowledgeTrainingConditioning
{
    static constexpr int version = 1;

    std::string sampleId;
    CompositionComposerKnowledgeSampleRepresentation composerKnowledge;
    CompositionGenreKnowledgeRepresentation genreKnowledge;
    CompositionSoundEngineeringKnowledgeRepresentation soundEngineeringKnowledge;

    bool valid = false;

    bool isValid() const noexcept;
};

struct CompositionKnowledgeTrainingSample
{
    static constexpr int version = 1;
    static constexpr std::size_t conditioningFeatureCount =
        CompositionComposerKnowledgeRepresentation::featureCount +
        CompositionGenreKnowledgeRepresentation::featureCount +
        CompositionSoundEngineeringKnowledgeRepresentation::featureCount;

    std::string sampleId;
    CompositionSequenceMetadata metadata;
    CompositionMidiTrainingSequence sequence;
    CompositionComposerKnowledgeSampleRepresentation composerKnowledge;
    CompositionGenreKnowledgeRepresentation genreKnowledge;
    CompositionSoundEngineeringKnowledgeRepresentation soundEngineeringKnowledge;
    std::array<double, conditioningFeatureCount> conditioningFeatures{};

    bool valid = false;

    bool isValid() const noexcept;
};

struct CompositionKnowledgeTrainingDataset
{
    static constexpr int version = 1;

    std::size_t featureWidth =
        CompositionKnowledgeTrainingSample::conditioningFeatureCount;
    std::vector<CompositionKnowledgeTrainingSample> samples;

    bool verified = false;
    bool valid = false;

    bool isValid() const noexcept;
    std::size_t sampleCount() const noexcept;
};

CompositionKnowledgeTrainingDataset
buildCompositionKnowledgeTrainingDataset(
    const CompositionConditionedTrainingDataset& conditionedDataset,
    const std::vector<CompositionKnowledgeTrainingConditioning>& conditionings) noexcept;

} // namespace midigengx::music
