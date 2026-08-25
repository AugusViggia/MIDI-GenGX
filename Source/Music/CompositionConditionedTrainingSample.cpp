#include "CompositionConditionedTrainingSample.h"

namespace midigengx::music
{

bool CompositionConditionedTrainingSample::isValid()
    const noexcept
{
    return valid &&
           sequence.isValid() &&
           metadata.isValid() &&
           sequence.sampleId ==
               metadata.sampleId;
}

} // namespace midigengx::music
