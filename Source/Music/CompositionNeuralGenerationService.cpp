#include "CompositionNeuralGenerationService.h"

namespace midigengx::music
{

bool CompositionNeuralGenerationResult::isValid(
    std::size_t expectedWidth)
    const noexcept
{
    if (!valid ||
        requestedSectionCount == 0 ||
        generatedSections.size() !=
            requestedSectionCount)
    {
        return false;
    }

    for (const auto& section :
         generatedSections)
    {
        if (section.size() !=
                expectedWidth)
        {
            return false;
        }
    }

    return true;
}

CompositionNeuralGenerationResult
generateNextSections(
    const CompositionInferencePipeline& pipeline,
    const std::vector<double>& globalFeatures,
    const std::vector<double>& initialContextSection,
    std::size_t sectionCount)
    noexcept
{
    CompositionNeuralGenerationResult result;

    if (!pipeline.isValid() ||
        globalFeatures.size() !=
            pipeline.contract.globalInputWidth ||
        initialContextSection.size() !=
            pipeline.contract.sectionInputWidth ||
        sectionCount == 0)
    {
        return result;
    }

    std::vector<double> context =
        initialContextSection;

    result.requestedSectionCount =
        sectionCount;

    result.generatedSections.reserve(
        sectionCount);

    for (std::size_t index = 0;
         index < sectionCount;
         ++index)
    {
        CompositionInferenceRequest request;

        request.globalFeatures =
            globalFeatures;

        request.contextSectionFeatures =
            context;

        request.contextIsValid =
            true;

        const auto inference =
            pipeline.infer(
                request);

        if (!inference.isValid(
                pipeline.contract))
        {
            return CompositionNeuralGenerationResult{};
        }

        const auto& generated =
            inference.prediction.sectionFeatures;

        result.generatedSections.push_back(
            generated);

        context =
            generated;
    }

    result.valid =
        true;

    return result;
}

} // namespace midigengx::music
