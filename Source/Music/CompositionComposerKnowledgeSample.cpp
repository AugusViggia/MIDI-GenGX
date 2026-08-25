#include "CompositionComposerKnowledgeSample.h"

namespace midigengx::music
{

bool CompositionComposerKnowledgeSample::isValid()
    const noexcept
{
    return valid &&
           composition.isValid() &&
           metadata.isValid() &&
           composition.sampleId ==
               metadata.sampleId;
}

CompositionComposerKnowledgeSample
buildCompositionComposerKnowledgeSample(
    const CompositionDatasetSample& composition,
    const CompositionSequenceMetadata& metadata)
    noexcept
{
    CompositionComposerKnowledgeSample result;

    if (!composition.isValid() ||
        !metadata.isValid() ||
        composition.sampleId !=
            metadata.sampleId)
    {
        return result;
    }

    result.composition =
        composition;

    result.metadata =
        metadata;

    result.valid =
        true;

    return result;
}

} // namespace midigengx::music
