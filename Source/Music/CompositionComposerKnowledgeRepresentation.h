#pragma once

#include "CompositionComposerKnowledgeCatalog.h"

#include <array>
#include <cstddef>
#include <string>
#include <vector>

namespace midigengx::music
{

struct CompositionComposerKnowledgeRepresentation
{
    static constexpr int version = 1;
    static constexpr std::size_t featureCount = 14;

    enum Feature : std::size_t
    {
        MajorHarmonyRatio = 0,
        MinorHarmonyRatio,
        DiminishedHarmonyRatio,
        AugmentedHarmonyRatio,
        SuspendedHarmonyRatio,
        UnknownHarmonyRatio,
        OpeningPhraseRatio,
        DevelopmentPhraseRatio,
        PreparationPhraseRatio,
        CadencePhraseRatio,
        AverageSectionTension,
        PeakSectionTension,
        RisingTransitionRatio,
        FallingTransitionRatio
    };

    std::string composerId;
    std::size_t sampleCount = 0;

    // Fixed-width, deterministic composer-language representation.
    // Every value is normalized to [0, 1]. Ordering is part of version 1.
    std::array<double, featureCount> features{};

    bool valid = false;

    bool isValid() const noexcept;
};

struct CompositionComposerKnowledgeSampleRepresentation
{
    static constexpr int version = 1;

    std::string sampleId;
    std::string composerId;
    std::array<double, CompositionComposerKnowledgeRepresentation::featureCount> features{};
    bool valid = false;

    bool isValid() const noexcept;
};

CompositionComposerKnowledgeSampleRepresentation
buildCompositionComposerKnowledgeSampleRepresentation(
    const CompositionComposerKnowledgeSample& sample) noexcept;

CompositionComposerKnowledgeRepresentation
buildCompositionComposerKnowledgeRepresentation(
    const CompositionComposerKnowledgeGroup& group) noexcept;

} // namespace midigengx::music
