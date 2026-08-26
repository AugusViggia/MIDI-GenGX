#include "CompositionConditionedSequenceNeuralModelArtifactFileInspector.h"

#include <fstream>
#include <iterator>
#include <sstream>
#include <vector>

namespace midigengx::music
{
namespace
{

std::uint32_t readU32(
    const std::vector<std::uint8_t>& bytes,
    const std::size_t offset) noexcept
{
    if (offset +
            sizeof(std::uint32_t) >
        bytes.size())
    {
        return 0;
    }

    return
        static_cast<std::uint32_t>(
            bytes[offset]) |
        (static_cast<std::uint32_t>(
             bytes[offset + 1]) << 8) |
        (static_cast<std::uint32_t>(
             bytes[offset + 2]) << 16) |
        (static_cast<std::uint32_t>(
             bytes[offset + 3]) << 24);
}

std::string join(
    const std::vector<std::string>& values)
{
    std::ostringstream stream;

    for (std::size_t index = 0;
         index < values.size();
         ++index)
    {
        if (index != 0)
            stream << ',';

        stream << values[index];
    }

    return stream.str();
}

} // namespace

bool CompositionConditionedSequenceNeuralModelArtifactInspectionResult::isValid()
    const noexcept
{
    return valid &&
           fileByteCount > 0 &&
           artifactVersion ==
               CompositionConditionedSequenceNeuralModelArtifact::version &&
           loadResult.isValid() &&
           !loadResult.model.vocabulary.composers.empty() &&
           !loadResult.model.vocabulary.styles.empty() &&
           !loadResult.model.vocabulary.eras.empty() &&
           !loadResult.model.vocabulary.instrumentations.empty();
}

CompositionConditionedSequenceNeuralModelArtifactInspectionResult
inspectCompositionConditionedSequenceNeuralModelArtifactFile(
    const std::string& filePath) noexcept
{
    CompositionConditionedSequenceNeuralModelArtifactInspectionResult result;

    if (filePath.empty())
        return result;

    std::ifstream file(
        filePath,
        std::ios::binary);

    if (!file)
        return result;

    const std::vector<std::uint8_t> bytes(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    result.fileByteCount =
        bytes.size();

    if (bytes.size() <
        sizeof(std::uint32_t) * 2)
    {
        return result;
    }

    result.artifactVersion =
        readU32(
            bytes,
            sizeof(std::uint32_t));

    result.loadResult =
        CompositionConditionedSequenceNeuralModelRuntimeLoader{}
            .load(bytes);

    if (!result.loadResult.isValid())
        return result;

    result.composerSummary =
        join(
            result.loadResult.model.vocabulary.composers);

    result.styleSummary =
        join(
            result.loadResult.model.vocabulary.styles);

    result.eraSummary =
        join(
            result.loadResult.model.vocabulary.eras);

    result.instrumentationSummary =
        join(
            result.loadResult.model.vocabulary.instrumentations);

    result.valid =
        true;

    return result;
}

} // namespace midigengx::music
