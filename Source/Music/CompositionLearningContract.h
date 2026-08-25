#pragma once

#include "CompositionDatasetPreparedView.h"

#include <cstddef>

namespace midigengx::music
{

enum class LearningObjective
{
    SectionStateReconstruction,
    NextSectionPrediction
};

struct CompositionLearningContract
{
    static constexpr int version = 1;

    LearningObjective objective =
        LearningObjective::NextSectionPrediction;

    std::size_t globalInputWidth =
        CompositionDatasetSchema::globalFeatureCount;

    std::size_t sectionInputWidth =
        CompositionDatasetSchema::sectionFeatureCount;

    std::size_t targetWidth =
        CompositionDatasetSchema::sectionFeatureCount;

    std::size_t contextLength = 0;

    bool usesSectionMask = true;
    bool analysisValid = false;

    bool isValid() const noexcept;
};

CompositionLearningContract
buildCompositionLearningContract(
    const CompositionDatasetPreparedView& prepared,
    LearningObjective objective =
        LearningObjective::NextSectionPrediction) noexcept;

} // namespace midigengx::music
