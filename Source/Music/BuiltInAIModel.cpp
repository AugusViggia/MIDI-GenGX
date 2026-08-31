#include "BuiltInAIModel.h"
#include <MIDI_GenGX_BuiltInAIModelData.h>

namespace midigengx::music::built_in_ai_model
{
const std::uint8_t* data() noexcept
{
    return reinterpret_cast<const std::uint8_t*>(
        MIDI_GenGX_BuiltInAIModelData::ChopinPhase122BStructuredLossR1_mgcn);
}

std::size_t size() noexcept
{
    return MIDI_GenGX_BuiltInAIModelData::ChopinPhase122BStructuredLossR1_mgcnSize;
}
}
