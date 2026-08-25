#include "Music/CompositionRealComposerCorpusPreparation.h"

#include <cstdlib>
#include <iostream>
#include <string>

using namespace midigengx::music;

namespace
{

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

CompositionSequenceMetadata makeMetadata(
    const std::string& id)
{
    CompositionSequenceMetadata metadata;

    metadata.sampleId = id;
    metadata.composerId = "chopin";
    metadata.workId = id;
    metadata.movementId = "single";
    metadata.styleId = "romantic";
    metadata.eraId = "romantic";
    metadata.instrumentationId = "solo_piano";
    metadata.verified = true;
    metadata.valid = true;

    return metadata;
}

CompositionSequenceMetadataCatalog makeMetadataCatalog()
{
    return buildCompositionSequenceMetadataCatalog(
    {
        makeMetadata("chopin-piece-a"),
        makeMetadata("chopin-piece-b")
    });
}

CompositionMidiCorpusRecord makeRecord(
    const std::string& sampleId,
    int pitchOffset)
{
    CompositionMidiCorpusRecord record;

    record.sampleId =
        sampleId;

    record.ticksPerQuarterNote =
        480;

    record.trackCount =
        1;

    constexpr std::uint32_t
        quarter = 480;

    for (std::size_t index = 0;
         index < 32;
         ++index)
    {
        CompositionMidiNote note;

        note.channel =
            0;

        constexpr int chordPattern[] =
        {
            0,
            4,
            7,
            12
        };

        note.midiNote =
            static_cast<std::uint8_t>(
                60 +
                pitchOffset +
                chordPattern[
                    index % 4]);

        note.velocity =
            96;

        note.startTick =
            static_cast<std::uint32_t>(
                index * quarter);

        note.endTick =
            note.startTick +
            quarter / 2;

        record.notes.push_back(
            note);

        record.lengthTicks =
            note.endTick;
    }

    record.analysisValid =
        true;

    return record;
}

void testRealComposerPreparationUsesFullMusicalEnrichment()
{
    const auto records =
        std::vector<CompositionMidiCorpusRecord>
        {
            makeRecord(
                "chopin-piece-a",
                0),
            makeRecord(
                "chopin-piece-b",
                2)
        };

    const auto result =
        prepareRealComposerCorpusFromRecords(
            records,
            makeMetadataCatalog());

    expect(
        result.isValid(),
        "real composer preparation is valid");

    expect(
        result.acceptedSampleCount == 2 &&
        result.rejectedSampleCount == 0,
        "all valid composer records are accepted");

    expect(
        result.conditionedDataset.sampleCount() == 2,
        "conditioned dataset contains every accepted composition");

    expect(
        result.conditionedDataset.verified,
        "conditioned real composer dataset is verified");
}

void testMissingMetadataIsRejected()
{
    auto records =
        std::vector<CompositionMidiCorpusRecord>
        {
            makeRecord(
                "chopin-piece-a",
                0)
        };

    auto metadata =
        makeMetadataCatalog();

    metadata.entries.erase(
        metadata.entries.begin());

    metadata.valid =
        true;

    const auto result =
        prepareRealComposerCorpusFromRecords(
            records,
            metadata);

    expect(
        !result.isValid(),
        "missing metadata prevents a valid composer corpus");

    expect(
        result.rejectedSampleCount == 1,
        "missing metadata sample is rejected");
}

void testInvalidMusicalRecordIsRejected()
{
    auto record =
        makeRecord(
            "chopin-piece-a",
            0);

    record.notes.clear();

    const auto result =
        prepareRealComposerCorpusFromRecords(
        {
            record
        },
        makeMetadataCatalog());

    expect(
        !result.isValid(),
        "invalid MIDI analysis is rejected");

    expect(
        result.rejectedSampleCount == 1,
        "invalid MIDI record increments rejection count");
}

} // namespace

int main()
{
    testRealComposerPreparationUsesFullMusicalEnrichment();
    testMissingMetadataIsRejected();
    testInvalidMusicalRecordIsRejected();

    std::cout
        << "MIDI-GenGX Phase 104 real composer corpus preparation tests passed.\n";

    return 0;
}
