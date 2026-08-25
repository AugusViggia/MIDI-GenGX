#include "CompositionComposerCorpusSourceManifest.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <unordered_set>

namespace midigengx::music
{
namespace
{

namespace fs = std::filesystem;

std::uint64_t hashBytes(
    const std::vector<std::uint8_t>& bytes) noexcept
{
    std::uint64_t hash =
        14695981039346656037ull;

    for (const auto byte :
         bytes)
    {
        hash ^=
            static_cast<std::uint64_t>(
                byte);

        hash *=
            1099511628211ull;
    }

    return hash;
}

bool isMidiExtension(
    const fs::path& path) noexcept
{
    auto extension =
        path.extension().string();

    std::transform(
        extension.begin(),
        extension.end(),
        extension.begin(),
        [](unsigned char value)
        {
            return static_cast<char>(
                std::tolower(value));
        });

    return extension == ".mid" ||
           extension == ".midi";
}

bool lessPath(
    const fs::path& left,
    const fs::path& right) noexcept
{
    const auto normalize =
        [](const fs::path& value)
        {
            auto text =
                value.lexically_normal().generic_string();

            std::transform(
                text.begin(),
                text.end(),
                text.begin(),
                [](unsigned char character)
                {
                    return static_cast<char>(
                        std::tolower(character));
                });

            return text;
        };

    return normalize(left) <
           normalize(right);
}

std::string sampleIdForPath(
    const fs::path& root,
    const fs::path& path)
{
    auto relative =
        path.lexically_relative(
            root).lexically_normal();

    relative.replace_extension();

    return relative.generic_string();
}

bool readFile(
    const fs::path& path,
    std::vector<std::uint8_t>& bytes) noexcept
{
    std::ifstream file(
        path,
        std::ios::binary);

    if (!file)
        return false;

    file.seekg(
        0,
        std::ios::end);

    const auto size =
        file.tellg();

    if (size < 0)
        return false;

    file.seekg(
        0,
        std::ios::beg);

    bytes.resize(
        static_cast<std::size_t>(
            size));

    if (!bytes.empty())
    {
        file.read(
            reinterpret_cast<char*>(
                bytes.data()),
            static_cast<std::streamsize>(
                bytes.size()));

        if (!file)
            return false;
    }

    return true;
}

} // namespace

bool CompositionComposerCorpusSourceEntry::isValid()
    const noexcept
{
    return valid &&
           !sampleId.empty() &&
           !relativeMidiPath.empty() &&
           byteSize > 0 &&
           contentHash != 0;
}

bool CompositionComposerCorpusSourceManifest::isValid()
    const noexcept
{
    if (!valid ||
        corpusRoot.empty() ||
        entries.empty())
    {
        return false;
    }

    std::unordered_set<std::string> sampleIds;
    std::unordered_set<std::string> paths;

    for (const auto& entry :
         entries)
    {
        if (!entry.isValid() ||
            !sampleIds.insert(
                entry.sampleId).second ||
            !paths.insert(
                entry.relativeMidiPath).second)
        {
            return false;
        }
    }

    return true;
}

std::size_t
CompositionComposerCorpusSourceManifest::sampleCount()
    const noexcept
{
    return entries.size();
}

CompositionComposerCorpusSourceManifest
buildCompositionComposerCorpusSourceManifest(
    const std::string& midiDirectoryPath,
    const CompositionComposerKnowledgeCatalog& catalog,
    bool recursive) noexcept
{
    CompositionComposerCorpusSourceManifest result;

    if (!catalog.isValid() ||
        midiDirectoryPath.empty())
    {
        return result;
    }

    const fs::path root =
        fs::path(
            midiDirectoryPath)
            .lexically_normal();

    std::error_code error;

    if (!fs::exists(root, error) ||
        !fs::is_directory(root, error))
    {
        return result;
    }

    std::vector<fs::path> files;

    if (recursive)
    {
        for (const auto& item :
             fs::recursive_directory_iterator(
                 root,
                 fs::directory_options::skip_permission_denied,
                 error))
        {
            if (!error &&
                item.is_regular_file(error) &&
                isMidiExtension(
                    item.path()))
            {
                files.push_back(
                    item.path());
            }
        }
    }
    else
    {
        for (const auto& item :
             fs::directory_iterator(
                 root,
                 fs::directory_options::skip_permission_denied,
                 error))
        {
            if (!error &&
                item.is_regular_file(error) &&
                isMidiExtension(
                    item.path()))
            {
                files.push_back(
                    item.path());
            }
        }
    }

    std::sort(
        files.begin(),
        files.end(),
        lessPath);

    if (files.size() !=
        catalog.sampleCount())
    {
        return result;
    }

    std::unordered_set<std::string> catalogIds;

    for (const auto& composer :
         catalog.composers)
    {
        for (const auto& sample :
             composer.samples)
        {
            catalogIds.insert(
                sample.composition.sampleId);
        }
    }

    for (const auto& file :
         files)
    {
        const auto sampleId =
            sampleIdForPath(
                root,
                file);

        if (catalogIds.find(sampleId) ==
            catalogIds.end())
        {
            return result;
        }

        std::vector<std::uint8_t> bytes;

        if (!readFile(
                file,
                bytes))
        {
            return result;
        }

        CompositionComposerCorpusSourceEntry entry;

        entry.sampleId =
            sampleId;

        entry.relativeMidiPath =
            file.lexically_relative(
                    root)
                .lexically_normal()
                .generic_string();

        entry.byteSize =
            bytes.size();

        entry.contentHash =
            hashBytes(bytes);

        entry.valid =
            true;

        result.entries.push_back(
            std::move(entry));
    }

    result.corpusRoot =
        root.generic_string();

    result.valid =
        true;

    if (!result.isValid())
        result.valid = false;

    return result;
}

} // namespace midigengx::music
