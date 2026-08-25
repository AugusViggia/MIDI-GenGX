#pragma once

#include "CompositionComposerKnowledgeCorpusAssembly.h"

#include <cstddef>
#include <string>
#include <vector>

namespace midigengx::music
{

struct CompositionRealComposerCorpusIntakeIssue
{
    std::string sampleId;
    std::string relativeMidiPath;
    std::string message;

    bool isValid() const noexcept;
};

struct CompositionRealComposerCorpusIntakeReport
{
    static constexpr int version = 1;

    std::string corpusRoot;
    std::string composerId;

    std::size_t discoveredMidiFiles = 0;
    std::size_t expectedSamples = 0;
    std::size_t matchedSamples = 0;
    std::size_t rejectedSamples = 0;

    std::vector<CompositionRealComposerCorpusIntakeIssue> issues;

    bool valid = false;

    bool isValid() const noexcept;
};

CompositionRealComposerCorpusIntakeReport
inspectRealComposerCorpusDirectory(
    const std::string& midiDirectoryPath,
    const std::string& composerId,
    const CompositionComposerKnowledgeCatalog& catalog,
    bool recursive) noexcept;

bool canEnterFirstComposerTraining(
    const CompositionRealComposerCorpusIntakeReport& intake,
    const CompositionComposerKnowledgeCorpusAssembly& assembly) noexcept;

} // namespace midigengx::music
