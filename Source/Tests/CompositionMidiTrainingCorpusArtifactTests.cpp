#include "Music/CompositionMidiTrainingCorpusArtifact.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <vector>

using namespace midigengx::music;

namespace
{

void expect(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(1);
    }
}

CompositionMidiTrainingSequence buildSequence(
    const std::string& id,
    double offset)
{
    CompositionMidiTrainingSequence sequence;
    sequence.sampleId = id;
    sequence.featureWidth =
        CompositionMidiTrainingEvent::featureCount;
    sequence.analysisValid = true;

    for (int index = 0;
         index < 4;
         ++index)
    {
        CompositionMidiTrainingEvent event;

        event.features.resize(
            CompositionMidiTrainingEvent::featureCount);

        for (std::size_t feature = 0;
             feature < event.features.size();
             ++feature)
        {
            event.features[feature] =
                std::clamp(
                    0.01 *
                        static_cast<double>(
                            feature) +
                    offset +
                    static_cast<double>(
                        index) *
                        0.001,
                    -1.0,
                    1.0);
        }

        sequence.events.push_back(
            event);
    }

    return sequence;
}

void testArtifactRoundTrip()
{
    std::vector<CompositionMidiTrainingSequence> sequences =
    {
        buildSequence("piece-a", 0.0),
        buildSequence("piece-b", 0.1),
        buildSequence("piece-c", -0.1)
    };

    const auto artifact =
        serializeCompositionMidiTrainingSequences(
            sequences);

    expect(
        artifact.isValid(),
        "training sequence corpus artifact is valid");

    std::vector<CompositionMidiTrainingSequence> restored;

    expect(
        deserializeCompositionMidiTrainingSequences(
            artifact,
            restored),
        "training sequence corpus artifact deserializes");

    expect(
        restored.size() == sequences.size(),
        "round trip preserves sequence count");

    for (std::size_t index = 0;
         index < sequences.size();
         ++index)
    {
        expect(
            restored[index].sampleId ==
                sequences[index].sampleId &&
            restored[index].events.size() ==
                sequences[index].events.size(),
            "round trip preserves sequence identity and event count");

        for (std::size_t eventIndex = 0;
             eventIndex < sequences[index].events.size();
             ++eventIndex)
        {
            expect(
                restored[index].events[eventIndex].features ==
                    sequences[index].events[eventIndex].features,
                "round trip preserves event features");
        }
    }
}

void testSerializationIsDeterministic()
{
    std::vector<CompositionMidiTrainingSequence> sequences =
    {
        buildSequence("det-a", 0.0),
        buildSequence("det-b", 0.2)
    };

    const auto first =
        serializeCompositionMidiTrainingSequences(
            sequences);

    const auto second =
        serializeCompositionMidiTrainingSequences(
            sequences);

    expect(
        first.isValid() &&
        second.isValid(),
        "deterministic sequence artifacts are valid");

    expect(
        first.bytes == second.bytes,
        "sequence corpus serialization is deterministic");
}

void testInvalidSequencesAreRejected()
{
    std::vector<CompositionMidiTrainingSequence> sequences;

    CompositionMidiTrainingSequence invalid;
    invalid.sampleId = "invalid";

    sequences.push_back(
        invalid);

    const auto artifact =
        serializeCompositionMidiTrainingSequences(
            sequences);

    expect(
        !artifact.isValid(),
        "invalid sequence cannot be serialized");
}

void testCorruptedArtifactIsRejected()
{
    auto sequence =
        buildSequence(
            "corrupt",
            0.0);

    auto artifact =
        serializeCompositionMidiTrainingSequences(
        {
            sequence
        });

    expect(
        artifact.isValid(),
        "baseline artifact for corruption test is valid");

    if (!artifact.bytes.empty())
    {
        artifact.bytes.back() ^= 0x7F;
    }

    std::vector<CompositionMidiTrainingSequence> restored;

    expect(
        !deserializeCompositionMidiTrainingSequences(
            artifact,
            restored),
        "corrupted artifact is rejected");
}

} // namespace

int main()
{
    testArtifactRoundTrip();
    testSerializationIsDeterministic();
    testInvalidSequencesAreRejected();
    testCorruptedArtifactIsRejected();

    std::cout
        << "MIDI-GenGX Phase 84 training corpus artifact tests passed.\n";

    return 0;
}
