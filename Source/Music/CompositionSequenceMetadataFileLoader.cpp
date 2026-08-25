#include "CompositionSequenceMetadataFileLoader.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <vector>

namespace midigengx::music
{
namespace
{

bool parseVerified(
    const std::string& rawValue,
    bool& verified) noexcept
{
    std::string value =
        rawValue;

    while (!value.empty() &&
           std::isspace(
               static_cast<unsigned char>(
                   value.back())))
    {
        value.pop_back();
    }

    if (value == "1" ||
        value == "true" ||
        value == "TRUE")
    {
        verified = true;
        return true;
    }

    if (value == "0" ||
        value == "false" ||
        value == "FALSE")
    {
        verified = false;
        return true;
    }

    return false;
}

bool parseLine(
    const std::string& line,
    CompositionSequenceMetadata& metadata) noexcept
{
    if (line.empty() ||
        line[0] == '#')
    {
        return false;
    }

    std::vector<std::string> fields;
    std::stringstream stream(line);
    std::string field;

    while (std::getline(
        stream,
        field,
        '\t'))
    {
        fields.push_back(field);
    }

    if (fields.size() != 8)
        return false;

    metadata.sampleId =
        fields[0];

    metadata.composerId =
        fields[1];

    metadata.workId =
        fields[2];

    metadata.movementId =
        fields[3];

    metadata.styleId =
        fields[4];

    metadata.eraId =
        fields[5];

    metadata.instrumentationId =
        fields[6];

    if (!parseVerified(
            fields[7],
            metadata.verified))
    {
        return false;
    }

    // Parsing has successfully populated the metadata contract. Mark the
    // structure as constructed before invoking its structural validator.
    metadata.valid =
        true;

    return metadata.isValid();
}

} // namespace

bool CompositionSequenceMetadataFileLoadResult::isValid()
    const noexcept
{
    return valid &&
           parsedEntryCount ==
               catalog.entries.size() &&
           parsedEntryCount > 0 &&
           rejectedLineCount == 0 &&
           catalog.isValid();
}

CompositionSequenceMetadataFileLoadResult
loadCompositionSequenceMetadataFile(
    const std::string& filePath)
    noexcept
{
    CompositionSequenceMetadataFileLoadResult result;

    if (filePath.empty())
        return result;

    std::ifstream file(
        filePath);

    if (!file)
        return result;

    std::vector<CompositionSequenceMetadata>
        entries;

    std::string line;

    while (std::getline(
        file,
        line))
    {
        if (line.empty() ||
            line[0] == '#')
        {
            continue;
        }

        CompositionSequenceMetadata metadata;

        if (!parseLine(
                line,
                metadata))
        {
            ++result.rejectedLineCount;
            continue;
        }

        entries.push_back(
            std::move(metadata));
    }

    if (entries.empty())
        return result;

    result.catalog =
        buildCompositionSequenceMetadataCatalog(
            entries);

    if (!result.catalog.isValid())
        return result;

    result.parsedEntryCount =
        entries.size();

    result.valid =
        true;

    if (!result.isValid())
        result.valid = false;

    return result;
}

} // namespace midigengx::music
