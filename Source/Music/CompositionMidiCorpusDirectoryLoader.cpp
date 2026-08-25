#include "CompositionMidiCorpusDirectoryLoader.h"

#include "CompositionMidiFileCorpusReader.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iterator>

namespace midigengx::music
{
namespace
{

namespace fs = std::filesystem;

std::string lowerAscii(
    std::string value)
{
    std::transform(
        value.begin(),
        value.end(),
        value.begin(),
        [](unsigned char character)
        {
            return static_cast<char>(
                std::tolower(
                    character));
        });

    return value;
}

bool isMidiExtension(
    const fs::path& path)
{
    const auto extension =
        lowerAscii(
            path.extension().string());

    return extension == ".mid" ||
           extension == ".midi";
}

bool readBinaryFile(
    const fs::path& path,
    std::vector<std::uint8_t>& bytes)
{
    std::ifstream file(
        path,
        std::ios::binary);

    if (!file)
        return false;

    file.seekg(
        0,
        std::ios::end);

    const auto end =
        file.tellg();

    if (end <= 0)
        return false;

    file.seekg(
        0,
        std::ios::beg);

    bytes.resize(
        static_cast<std::size_t>(
            end));

    file.read(
        reinterpret_cast<char*>(
            bytes.data()),
        static_cast<std::streamsize>(
            bytes.size()));

    return static_cast<std::size_t>(
               file.gcount()) ==
           bytes.size();
}

bool lessPath(
    const fs::path& left,
    const fs::path& right)
{
    return lowerAscii(
               left.lexically_normal().string()) <
           lowerAscii(
               right.lexically_normal().string());
}

std::vector<fs::path>
discoverMidiFiles(
    const fs::path& root,
    bool recursive)
{
    std::vector<fs::path> paths;

    std::error_code error;

    if (!fs::is_directory(
            root,
            error))
    {
        return paths;
    }

    if (recursive)
    {
        fs::recursive_directory_iterator iterator(
            root,
            fs::directory_options::skip_permission_denied,
            error);

        if (error)
            return paths;

        const fs::recursive_directory_iterator end;

        for (; iterator != end; iterator.increment(error))
        {
            if (error)
            {
                error.clear();
                continue;
            }

            if (!iterator->is_regular_file(
                    error))
            {
                error.clear();
                continue;
            }

            if (isMidiExtension(
                    iterator->path()))
            {
                paths.push_back(
                    iterator->path());
            }
        }
    }
    else
    {
        fs::directory_iterator iterator(
            root,
            fs::directory_options::skip_permission_denied,
            error);

        if (error)
            return paths;

        const fs::directory_iterator end;

        for (; iterator != end; iterator.increment(error))
        {
            if (error)
            {
                error.clear();
                continue;
            }

            if (!iterator->is_regular_file(
                    error))
            {
                error.clear();
                continue;
            }

            if (isMidiExtension(
                    iterator->path()))
            {
                paths.push_back(
                    iterator->path());
            }
        }
    }

    std::sort(
        paths.begin(),
        paths.end(),
        lessPath);

    return paths;
}

std::string sampleIdForPath(
    const fs::path& root,
    const fs::path& path)
{
    // Use a purely lexical relative path here. `std::filesystem::relative`
    // may resolve through the filesystem and produce platform-dependent
    // results for temporary/network/symlinked roots. The corpus identity must
    // be a deterministic function of the discovered path.
    auto relative =
        path.lexically_relative(
            root).lexically_normal();

    if (relative.empty())
    {
        relative =
            path.filename().lexically_normal();
    }

    relative.replace_extension();

    auto id =
        relative.generic_string();

    if (id.empty())
        id = path.stem().generic_string();

    return id;
}

} // namespace

bool CompositionMidiCorpusDirectoryLoadResult::isValid()
    const noexcept
{
    return valid &&
           acceptedFileCount ==
               records.size() &&
           acceptedFileCount +
               rejectedFileCount ==
               discoveredFileCount;
}

CompositionMidiCorpusDirectoryLoadResult
loadCompositionMidiCorpusDirectory(
    const std::string& directoryPath,
    bool recursive)
    noexcept
{
    CompositionMidiCorpusDirectoryLoadResult result;

    if (directoryPath.empty())
        return result;

    const std::filesystem::path root =
        std::filesystem::path(
            directoryPath);

    std::error_code error;

    if (!std::filesystem::is_directory(
            root,
            error))
    {
        return result;
    }

    const auto files =
        discoverMidiFiles(
            root,
            recursive);

    result.discoveredFileCount =
        files.size();

    CompositionMidiFileCorpusReader reader;

    result.records.reserve(
        files.size());

    for (const auto& path :
         files)
    {
        std::vector<std::uint8_t> bytes;

        if (!readBinaryFile(
                path,
                bytes))
        {
            ++result.rejectedFileCount;
            continue;
        }

        const auto sampleId =
            sampleIdForPath(
                root,
                path);

        auto record =
            reader.read(
                sampleId,
                bytes.data(),
                bytes.size());

        if (!record.isValid())
        {
            ++result.rejectedFileCount;
            continue;
        }

        result.records.push_back(
            std::move(record));

        ++result.acceptedFileCount;
    }

    result.valid =
        true;

    if (!result.isValid())
        result.valid = false;

    return result;
}

} // namespace midigengx::music
