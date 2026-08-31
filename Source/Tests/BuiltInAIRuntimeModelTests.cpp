#include "Plugin/PluginProcessor.h"
#include "Music/BuiltInAIModel.h"

#include <cassert>

int main()
{
    assert(midigengx::music::built_in_ai_model::data() != nullptr);
    assert(midigengx::music::built_in_ai_model::size() > 0);

    MIDIGenGXAudioProcessor processor;

    assert(processor.hasLoadedAIRuntimeModel());
    assert(processor.isAIRuntimeEnabled());

    return 0;
}
