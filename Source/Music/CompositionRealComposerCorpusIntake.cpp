#include "CompositionRealComposerCorpusIntake.h"

#include <algorithm>
#include <filesystem>
#include <unordered_set>
#include <vector>

namespace midigengx::music
{
namespace
{

namespace fs = std::filesystem;

std::string lowerExtension(
    const fs::path& path)
{
    auto value =
        path.extension().string();

    std::transform(
        value.begin(),
        value.end(),
        value.begin(),
        [](unsigned char character)
        {
            return static_cast<char>(
                std::tolower(character));
        });

    return value;
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

std::vector<fs::path> discoverMidiFiles(
    const fs::path& root,
    bool recursive)
{
    std::vector<fs::path> result;
    std::error_code error;

    if (!fs::is_directory(root, error))
        return result;

    if (recursive)
    {
        for (const auto& entry :
             fs::recursive_directory_iterator(
                 root,
                 fs::directory_options::skip_permission_denied,
                 error))
        {
            if (error)
            {
                error.clear();
                continue;
            }

            if (entry.is_regular_file(error) &&
                (lowerExtension(entry.path()) ==
                     ".mid" ||
                 lowerExtension(entry.path()) ==
                     ".midi"))
            {
                result.push_back(entry.path());
            }

            error.clear();
        }
    }
    else
    {
        for (const auto& entry :
             fs::directory_iterator(
                 root,
                 fs::directory_options::skip_permission_denied,
                 error))
        {
            if (error)
            {
                error.clear();
                continue;
            }

            if (entry.is_regular_file(error) &&
                (lowerExtension(entry.path()) ==
                     ".mid" ||
                 lowerExtension(entry.path()) ==
                     ".midi"))
            {
                result.push_back(entry.path());
            }

            error.clear();
        }
    }

    std::sort(
        result.begin(),
        result.end());

    return result;
}

} // namespace

bool CompositionRealComposerCorpusIntakeIssue::isValid()
    const noexcept
{
    return !message.empty();
}

bool CompositionRealComposerCorpusIntakeReport::isValid()
    const noexcept
{
    return valid &&
           !corpusRoot.empty() &&
           !composerId.empty() &&
           expectedSamples > 0 &&
           discoveredMidiFiles ==
               expectedSamples &&
           matchedSamples ==
               expectedSamples &&
           rejectedSamples == 0 &&
           issues.empty();
}

CompositionRealComposerCorpusIntakeReport
inspectRealComposerCorpusDirectory(
    const std::string& midiDirectoryPath,
    const std::string& composerId,
    const CompositionComposerKnowledgeCatalog& catalog,
    bool recursive) noexcept
{
    CompositionRealComposerCorpusIntakeReport report;

    if (midiDirectoryPath.empty() ||
        composerId.empty() ||
        !catalog.isValid())
    {
        return report;
    }

    const auto* composer =
        catalog.findComposer(
            composerId);

    if (composer == nullptr)
        return report;

    const fs::path root =
        fs::path(
            midiDirectoryPath)
            .lexically_normal();

    std::error_code error;

    if (!fs::is_directory(
            root,
            error))
    {
        return report;
    }

    const auto files =
        discoverMidiFiles(
            root,
            recursive);

    report.corpusRoot =
        root.generic_string();

    report.composerId =
        composerId;

    report.discoveredMidiFiles =
        files.size();

    report.expectedSamples =
        composer->sampleCount();

    std::unordered_set<std::string> expectedIds;

    for (const auto& sample :
         composer->samples)
    {
        expectedIds.insert(
            sample.composition.sampleId);
    }

    for (const auto& file :
         files)
    {
        const auto sampleId =
            sampleIdForPath(
                root,
                file);

        if (expectedIds.find(sampleId) ==
            expectedIds.end())
        {
            ++report.rejectedSamples;

            report.issues.push_back(
            {
                sampleId,
                file.lexically_relative(
                        root)
                    .lexically_normal()
                    .generic_string(),
                "MIDI source does not have a matching composer catalog sample ID."
            });
        }
        else
        {
            ++report.matchedSamples;
        }
    }

    for (const auto& sample :
         composer->samples)
    {
        if (std::none_of(
                files.begin(),
                files.end(),
                [&](const auto& file)
                {
                    return sampleIdForPath(
                               root,
                               file) ==
                           sample.composition.sampleId;
                }))
        {
            ++report.rejectedSamples;

            report.issues.push_back(
            {
                sample.composition.sampleId,
                std::string {},
                "Catalog sample does not have a matching MIDI source file."
            });
        }
    }

    report.valid = true;

    if (!report.isValid())
        report.valid = false;

    return report;
}

bool canEnterFirstComposerTraining(
    const CompositionRealComposerCorpusIntakeReport& intake,
    const CompositionComposerKnowledgeCorpusAssembly& assembly)
    noexcept
{
    return intake.isValid() &&
           assembly.isValid() &&
           intake.composerId.size() > 0;
}

} // namespace midigengx::music
