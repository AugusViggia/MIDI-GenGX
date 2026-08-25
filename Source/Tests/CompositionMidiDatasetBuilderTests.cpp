#include "Music/CompositionMidiDatasetBuilder.h"

#include <cstdlib>
#include <iostream>
#include <vector>

using namespace midigengx::music;

namespace
{

void expect(
    bool condition,
    const char* message)
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

CompositionMidiCorpusRecord buildRecord(
    const std::string& id,
    int pitchOffset)
{
    CompositionMidiCorpusRecord record;

    record.sampleId =
        id;

    record.ticksPerQuarterNote =
        480;

    record.trackCount =
        1;

    record.lengthTicks =
        64 * 480;

    record.analysisValid =
        true;

    const int pattern[4] =
    {
        0, 2, 4, 2
    };

    for (int section = 0;
         section < 4;
         ++section)
    {
        const auto sectionStart =
            static_cast<std::uint32_t>(
                section * 16 * 480);

        for (int index = 0;
             index < 16;
             ++index)
        {
            const auto start =
                sectionStart +
                static_cast<std::uint32_t>(
                    index * 480);

            record.notes.push_back(
            {
                0,
                static_cast<std::uint8_t>(
                    60 +
                    pitchOffset +
                    pattern[index % 4]),
                96,
                start,
                start + 240
            });
        }
    }

    return record;
}

void testSingleMidiRecordBuildsDataset()
{
    const std::vector<CompositionMidiCorpusRecord> records =
    {
        buildRecord(
            "song-a",
            0)
    };

    const auto result =
        buildCompositionDatasetFromMidiCorpus(
            records);

    expect(
        result.isValid(),
        "single MIDI corpus build is valid");

    expect(
        result.dataset.size() == 1,
        "single MIDI record creates one dataset sample");

    expect(
        result.acceptedCount == 1 &&
        result.rejectedCount == 0,
        "single valid MIDI record is accepted");
}

void testMultipleMidiRecordsProduceSortedDataset()
{
    const std::vector<CompositionMidiCorpusRecord> records =
    {
        buildRecord(
            "song-c",
            0),
        buildRecord(
            "song-a",
            2),
        buildRecord(
            "song-b",
            5)
    };

    const auto result =
        buildCompositionDatasetFromMidiCorpus(
            records);

    expect(
        result.isValid(),
        "multi-record MIDI corpus build is valid");

    expect(
        result.dataset.size() == 3,
        "three MIDI records create three samples");

    expect(
        result.dataset.samples[0].sampleId == "song-a" &&
        result.dataset.samples[1].sampleId == "song-b" &&
        result.dataset.samples[2].sampleId == "song-c",
        "dataset samples are sorted deterministically by id");
}

void testDuplicateIdsAreRejected()
{
    const std::vector<CompositionMidiCorpusRecord> records =
    {
        buildRecord(
            "duplicate",
            0),
        buildRecord(
            "duplicate",
            7)
    };

    const auto result =
        buildCompositionDatasetFromMidiCorpus(
            records);

    expect(
        result.isValid(),
        "duplicate-id build remains structurally valid");

    expect(
        result.dataset.size() == 1,
        "duplicate id does not create a second sample");

    expect(
        result.acceptedCount == 1 &&
        result.rejectedCount == 1,
        "duplicate id is counted as rejected");
}

void testInvalidRecordsAreRejected()
{
    auto invalid =
        buildRecord(
            "invalid",
            0);

    invalid.notes.clear();

    const std::vector<CompositionMidiCorpusRecord> records =
    {
        invalid,
        buildRecord(
            "valid",
            0)
    };

    const auto result =
        buildCompositionDatasetFromMidiCorpus(
            records);

    expect(
        result.isValid(),
        "mixed valid/invalid corpus build is valid");

    expect(
        result.acceptedCount == 1 &&
        result.rejectedCount == 1,
        "invalid MIDI record is rejected");
}

void testFeatureWidthsMatchSchema()
{
    const std::vector<CompositionMidiCorpusRecord> records =
    {
        buildRecord(
            "schema",
            0)
    };

    const auto result =
        buildCompositionDatasetFromMidiCorpus(
            records);

    expect(
        result.isValid(),
        "schema build is valid");

    const auto* sample =
        result.dataset.findById(
            "schema");

    expect(
        sample != nullptr,
        "schema sample exists");

    expect(
        sample->globalFeatures.size() == 13,
        "global feature width remains 13");

    for (const auto& section :
         sample->sectionFeatures)
    {
        expect(
            section.size() == 6,
            "section feature width remains 6");
    }
}

void testBuildIsDeterministic()
{
    const std::vector<CompositionMidiCorpusRecord> records =
    {
        buildRecord("det-b", 0),
        buildRecord("det-a", 3)
    };

    const auto first =
        buildCompositionDatasetFromMidiCorpus(
            records);

    const auto second =
        buildCompositionDatasetFromMidiCorpus(
            records);

    expect(
        first.isValid() &&
        second.isValid(),
        "deterministic corpus builds are valid");

    expect(
        first.dataset.samples.size() ==
            second.dataset.samples.size(),
        "deterministic corpus size is preserved");

    for (std::size_t index = 0;
         index < first.dataset.samples.size();
         ++index)
    {
        expect(
            first.dataset.samples[index].globalFeatures ==
                second.dataset.samples[index].globalFeatures &&
            first.dataset.samples[index].sectionFeatures ==
                second.dataset.samples[index].sectionFeatures,
            "identical MIDI corpus builds produce identical features");
    }
}

} // namespace

int main()
{
    testSingleMidiRecordBuildsDataset();
    testMultipleMidiRecordsProduceSortedDataset();
    testDuplicateIdsAreRejected();
    testInvalidRecordsAreRejected();
    testFeatureWidthsMatchSchema();
    testBuildIsDeterministic();

    std::cout
        << "MIDI-GenGX MIDI dataset builder tests passed.\n";

    return 0;
}
