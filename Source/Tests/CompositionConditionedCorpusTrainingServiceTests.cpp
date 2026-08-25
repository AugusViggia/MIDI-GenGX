#include "Music/CompositionConditionedCorpusTrainingService.h"
#include "Music/CompositionMidiCorpusDirectoryLoader.h"
#include "Music/CompositionRealComposerCorpusPreparation.h"
#include "Music/CompositionConditionedTrainingDataset.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace midigengx::music;

namespace
{
namespace fs = std::filesystem;

void expect(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr
            << "FAILED: "
            << message
            << '\n';
        std::exit(1);
    }
}

void appendU32BE(
    std::vector<std::uint8_t>& bytes,
    std::uint32_t value)
{
    bytes.push_back(
        static_cast<std::uint8_t>(
            value >> 24));
    bytes.push_back(
        static_cast<std::uint8_t>(
            value >> 16));
    bytes.push_back(
        static_cast<std::uint8_t>(
            value >> 8));
    bytes.push_back(
        static_cast<std::uint8_t>(
            value));
}

std::vector<std::uint8_t> buildMidi(
    int root)
{
    std::vector<std::uint8_t> bytes =
    {
        'M','T','h','d'
    };

    appendU32BE(bytes, 6);

    bytes.push_back(0);
    bytes.push_back(0);

    bytes.push_back(0);
    bytes.push_back(1);

    bytes.push_back(0x01);
    bytes.push_back(0xE0);

    std::vector<std::uint8_t> track;

    constexpr int intervals[3] =
    {
        0, 4, 7
    };

    for (int beat = 0;
         beat < 64;
         ++beat)
    {
        for (int note = 0;
             note < 3;
             ++note)
        {
            track.push_back(0x00);
            track.push_back(0x90);
            track.push_back(
                static_cast<std::uint8_t>(
                    root + intervals[note]));
            track.push_back(96);
        }

        for (int note = 0;
             note < 3;
             ++note)
        {
            if (note == 0)
            {
                track.push_back(0x83);
                track.push_back(0x60);
            }
            else
            {
                track.push_back(0x00);
            }

            track.push_back(0x80);
            track.push_back(
                static_cast<std::uint8_t>(
                    root + intervals[note]));
            track.push_back(0);
        }
    }

    track.push_back(0x00);
    track.push_back(0xFF);
    track.push_back(0x2F);
    track.push_back(0x00);

    bytes.push_back('M');
    bytes.push_back('T');
    bytes.push_back('r');
    bytes.push_back('k');

    appendU32BE(
        bytes,
        static_cast<std::uint32_t>(
            track.size()));

    bytes.insert(
        bytes.end(),
        track.begin(),
        track.end());

    return bytes;
}

fs::path createCorpus()
{
    const auto root =
        fs::temp_directory_path() /
        "MIDI_GenGX_Phase94_Corpus";

    std::error_code error;

    fs::remove_all(
        root,
        error);

    fs::create_directories(
        root / "Nested",
        error);

    const auto files =
        {
            root / "chopin-piece.mid",
            root / "bach-piece.mid",
            root / "Nested" / "nested-piece.midi"
        };

    int rootPitch = 60;

    for (const auto& filePath :
         files)
    {
        fs::create_directories(
            filePath.parent_path(),
            error);

        std::ofstream file(
            filePath,
            std::ios::binary);

        const auto bytes =
            buildMidi(
                rootPitch++);

        file.write(
            reinterpret_cast<const char*>(
                bytes.data()),
            static_cast<std::streamsize>(
                bytes.size()));
    }

    return root;
}

CompositionSequenceMetadata makeMetadata(
    const std::string& id,
    const std::string& composer,
    const std::string& style,
    const std::string& era)
{
    CompositionSequenceMetadata metadata;

    metadata.sampleId = id;
    metadata.composerId = composer;
    metadata.workId = id + "_work";
    metadata.movementId = "single";
    metadata.styleId = style;
    metadata.eraId = era;
    metadata.instrumentationId = "solo_piano";
    metadata.verified = true;
    metadata.valid = true;

    return metadata;
}

CompositionSequenceMetadataCatalog buildMetadata(
    const fs::path& root)
{
    return buildCompositionSequenceMetadataCatalog(
        {
            makeMetadata(
                "bach-piece",
                "bach",
                "baroque",
                "baroque"),
            makeMetadata(
                "chopin-piece",
                "chopin",
                "romantic",
                "romantic"),
            makeMetadata(
                "Nested/nested-piece",
                "chopin",
                "romantic",
                "romantic")
        });
}

void testRealDirectoryToTraining()
{
    const auto root =
        createCorpus();

    const auto loader =
        loadCompositionMidiCorpusDirectory(
            root.string(),
            true);

    expect(
        loader.isValid(),
        "real corpus loader is valid");

    bool foundNested =
        false;

    for (const auto& record :
         loader.records)
    {
        if (record.sampleId ==
            "Nested/nested-piece")
        {
            foundNested =
                true;
            break;
        }
    }

    expect(
        foundNested,
        "nested MIDI receives a stable relative sample ID");

    const auto metadata =
        buildMetadata(
            root);

    CompositionConditionedSequenceNeuralTrainingConfig config;
    config.epochs = 8;
    config.learningRate = 0.001;
    config.gradientClip = 1.0;

    const auto result =
        runCompositionConditionedCorpusTraining(
            root.string(),
            true,
            metadata,
            config);

    expect(
        result.isValid(),
        "real MIDI directory reaches conditioned neural training");

    expect(
        result.inputFileCount == 3 &&
        result.sequenceCount == 3 &&
        result.rejectedFileCount == 0,
        "all valid MIDI files become conditioned training samples");

    expect(
        result.trainingRun.training.finalLoss <
            result.trainingRun.training.initialLoss,
        "real corpus conditioned training reduces loss");

    expect(
        result.sequenceCorpusArtifact.isValid() &&
        result.metadataArtifact.isValid() &&
        result.trainingRun.modelArtifact.isValid(),
        "real corpus training artifacts are valid");

    std::error_code error;
    fs::remove_all(root,error);
}



void testServiceConsumesRealComposerPreparationPath()
{
    const auto root =
        createCorpus();

    const auto metadata =
        buildMetadata(root);

    CompositionConditionedSequenceNeuralTrainingConfig config;
    config.epochs = 4;
    config.learningRate = 0.001;
    config.gradientClip = 1.0;

    const auto prepared =
        prepareRealComposerCorpusFromDirectory(
            root.string(),
            true,
            metadata);

    expect(
        prepared.isValid(),
        "real composer preparation is valid before service execution");

    const auto result =
        runCompositionConditionedCorpusTraining(
            root.string(),
            true,
            metadata,
            config);

    expect(
        result.isValid(),
        "conditioned corpus training service accepts prepared real composer corpus");

    expect(
        result.sequenceCount ==
            prepared.acceptedSampleCount,
        "training service consumes every prepared sequence");

    expect(
        result.rejectedFileCount ==
            prepared.rejectedSampleCount,
        "training service preserves preparation rejection accounting");

    std::error_code error;
    fs::remove_all(root,error);
}

void testMissingMetadataFailsClosed()
{
    const auto root =
        createCorpus();

    const auto metadata =
        buildCompositionSequenceMetadataCatalog(
        {
            makeMetadata(
                "chopin-piece",
                "chopin",
                "romantic",
                "romantic")
        });

    CompositionConditionedSequenceNeuralTrainingConfig config;

    const auto result =
        runCompositionConditionedCorpusTraining(
            root.string(),
            true,
            metadata,
            config);

    expect(
        !result.valid,
        "incomplete metadata fails the real corpus training run");

    std::error_code error;
    fs::remove_all(root,error);
}

void testUnverifiedCatalogFailsClosed()
{
    const auto root =
        createCorpus();

    auto metadata =
        buildMetadata(
            root);

    metadata.entries[0].verified =
        false;

    const auto rebuilt =
        buildCompositionSequenceMetadataCatalog(
            metadata.entries);

    expect(
        rebuilt.isValid(),
        "unverified metadata catalog remains structurally valid");

    CompositionConditionedSequenceNeuralTrainingConfig config;

    const auto result =
        runCompositionConditionedCorpusTraining(
            root.string(),
            true,
            rebuilt,
            config);

    expect(
        !result.valid,
        "unverified metadata fails real corpus training");

    std::error_code error;
    fs::remove_all(root,error);
}

} // namespace

int main()
{
    testRealDirectoryToTraining();
    testServiceConsumesRealComposerPreparationPath();
    testMissingMetadataFailsClosed();
    testUnverifiedCatalogFailsClosed();

    std::cout
        << "MIDI-GenGX Phase 94 corpus training service tests passed.\n";

    return 0;
}
