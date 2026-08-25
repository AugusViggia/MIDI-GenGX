#include "CompositionAIMidiGenerationService.h"

#include <algorithm>
#include <cmath>

namespace midigengx::music
{
namespace
{

CompositionAIGuidance guidanceFromSection(
    const std::vector<double>& section,
    std::size_t expectedWidth) noexcept
{
    CompositionAIGuidance guidance;

    if (section.size() != expectedWidth ||
        expectedWidth != 6)
    {
        return guidance;
    }

    guidance.roleTarget =
        std::clamp(section[0], -1.0, 1.0);

    guidance.tensionTarget =
        std::clamp(section[1], -1.0, 1.0);

    guidance.tensionDeltaTarget =
        std::clamp(section[2], -1.0, 1.0);

    guidance.harmonyDegreeTarget =
        std::clamp(section[3], -1.0, 1.0);

    guidance.harmonyQualityTarget =
        std::clamp(section[4], -1.0, 1.0);

    guidance.harmonicDegreeDeltaTarget =
        std::clamp(section[5], -1.0, 1.0);

    guidance.confidence =
        1.0;

    guidance.valid = true;

    return guidance;
}

bool finiteVector(
    const std::vector<double>& values) noexcept
{
    for (const auto value :
         values)
    {
        if (!std::isfinite(value))
            return false;
    }

    return true;
}

} // namespace

bool CompositionAIMidiGenerationResult::isValid()
    const noexcept
{
    return valid &&
           requestedPhraseCount > 0 &&
           phrases.size() ==
               requestedPhraseCount;
}

CompositionAIMidiGenerationResult
generateAIMidiPhrases(
    const CompositionInferencePipeline& pipeline,
    const std::vector<double>& globalFeatures,
    const std::vector<double>& initialContextSection,
    const midigengx::domain::MusicalContext& baseContext,
    std::size_t phraseCount,
    std::uint32_t seed) noexcept
{
    CompositionAIMidiGenerationResult result;

    if (!pipeline.isValid() ||
        globalFeatures.size() !=
            pipeline.contract.globalInputWidth ||
        initialContextSection.size() !=
            pipeline.contract.sectionInputWidth ||
        phraseCount == 0 ||
        !finiteVector(globalFeatures) ||
        !finiteVector(initialContextSection))
    {
        return result;
    }

    const auto rollout =
        generateNextSections(
            pipeline,
            globalFeatures,
            initialContextSection,
            phraseCount);

    if (!rollout.isValid(
            pipeline.contract.targetWidth))
    {
        return result;
    }

    MusicalEngine engine;

    auto context =
        baseContext;

    context.normalize();

    result.requestedPhraseCount =
        phraseCount;

    result.phrases.reserve(
        phraseCount);

    for (std::size_t index = 0;
         index < rollout.generatedSections.size();
         ++index)
    {
        const auto guidance =
            guidanceFromSection(
                rollout.generatedSections[index],
                pipeline.contract.targetWidth);

        if (!guidance.isValid())
            return CompositionAIMidiGenerationResult{};

        auto phraseContext =
            context;

        // The existing MusicalEngine remains authoritative for hard musical
        // constraints. Generated AI features are used only as soft guidance.
        phraseContext.parameters.lengthBars =
            std::max(
                1,
                phraseContext.parameters.phraseLengthBars);

        phraseContext.normalize();

        const auto phrase =
            engine.generateWithAIGuidance(
                phraseContext,
                guidance,
                seed +
                    static_cast<std::uint32_t>(
                        index * 997u));

        if (!phrase.isValid())
            return CompositionAIMidiGenerationResult{};

        result.phrases.push_back(
            phrase);
    }

    result.valid =
        true;

    return result;
}

} // namespace midigengx::music
