#include "CompositionSequenceLearningContract.h"

namespace midigengx::music
{

bool CompositionSequenceLearningContract::isValid()
    const noexcept
{
    return analysisValid &&
           inputFeatureWidth ==
               CompositionMidiTrainingEvent::featureCount &&
           targetFeatureWidth ==
               CompositionMidiTrainingEvent::featureCount &&
           contextLength >= 4 &&
           contextLength <= 4096;
}

CompositionSequenceLearningContract
buildCompositionSequenceLearningContract(
    std::size_t contextLength,
    CompositionSequenceLearningObjective objective)
    noexcept
{
    CompositionSequenceLearningContract contract;

    contract.contextLength =
        contextLength;

    contract.objective =
        objective;

    contract.analysisValid =
        contract.contextLength >= 4 &&
        contract.contextLength <= 4096;

    return contract;
}

} // namespace midigengx::music
