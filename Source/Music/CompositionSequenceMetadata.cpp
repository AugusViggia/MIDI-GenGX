#include "CompositionSequenceMetadata.h"

namespace midigengx::music
{

bool CompositionSequenceMetadata::isValid()
    const noexcept
{
    return valid &&
           !sampleId.empty() &&
           !composerId.empty() &&
           !workId.empty() &&
           !styleId.empty() &&
           !eraId.empty() &&
           !instrumentationId.empty();
}

} // namespace midigengx::music
