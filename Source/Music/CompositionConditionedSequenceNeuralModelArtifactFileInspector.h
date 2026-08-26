#pragma once

#include "CompositionConditionedSequenceNeuralModelRuntimeLoader.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace midigengx::music
{

struct CompositionConditionedSequenceNeuralModelArtifactInspectionResult
{
    CompositionConditionedSequenceNeuralModelRuntimeLoadResult loadResult;

    std::size_t fileByteCount = 0;
    std::uint32_t artifactVersion = 0;

    std::string composerSummary;
    std::string styleSummary;
    std::string eraSummary;
    std::string instrumentationSummary;

    bool valid = false;

    bool isValid() const noexcept;
};

CompositionConditionedSequenceNeuralModelArtifactInspectionResult
inspectCompositionConditionedSequenceNeuralModelArtifactFile(
    const std::string& filePath) noexcept;

} // namespace midigengx::music
