#include "Music/CompositionMidiDatasetFeatureExtractor.h"

#include <cstdlib>
#include <iostream>

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

CompositionMidiCorpusRecord buildRecord()
{
    CompositionMidiCorpusRecord record;

    record.sampleId =
        "feature-extractor";

    record.ticksPerQuarterNote =
        480;

    record.trackCount =
        1;

    record.lengthTicks =
        64 * 480;

    record.analysisValid =
        true;

    for (int index = 0;
         index < 64;
         ++index)
    {
        const auto start =
            static_cast<std::uint32_t>(
                index * 480);

        const auto velocity =
            static_cast<std::uint8_t>(
                index >= 32
                    ? 110
                    : 60);

        record.notes.push_back(
        {
            0,
            static_cast<std::uint8_t>(
                index >= 32
                    ? 72
                    : 60),
            velocity,
            start,
            start + 240
        });

        if (index >= 32)
        {
            record.notes.push_back(
            {
                0,
                67,
                100,
                start,
                start + 180
            });
        }
    }

    return record;
}

void testRealMidiFeaturesBecomeSchemaSample()
{
    const auto record =
        buildRecord();

    const auto analysis =
        analyzeCompositionMidiCorpus(
            record);

    const auto sections =
        analyzeCompositionMidiSections(
            record);

    const auto harmony =
        analyzeCompositionMidiHarmony(
            record,
            sections);

    const auto motifs =
        analyzeCompositionMidiMotifs(
            record,
            sections);

    CompositionMidiDatasetFeatureExtractor extractor;

    const auto sample =
        extractor.buildSample(
            record,
            analysis,
            sections,
            harmony,
            motifs,
            "real-midi-001");

    expect(
        sample.isValid(),
        "real MIDI feature sample is valid");

    expect(
        sample.globalFeatures.size() == 13,
        "global feature width matches schema");

    expect(
        sample.sectionFeatures.size() ==
            sections.sectionCount(),
        "section count is preserved");

    for (const auto& section :
         sample.sectionFeatures)
    {
        expect(
            section.size() == 6,
            "section feature width matches schema");
    }

    expect(
        sample.globalFeatures[
            2] > 0.0,
        "real harmony count populates the global harmony feature");

    expect(
        sample.globalFeatures[
            3] >= 0.0 &&
        sample.globalFeatures[
            4] >= 0.0 &&
        sample.globalFeatures[
            5] >= 0.0,
        "real motif analysis populates motif feature slots");

    expect(
        sample.sectionFeatures.front()[3] >= 0.0 &&
        sample.sectionFeatures.front()[3] <= 1.0,
        "real harmony populates section scale-degree feature");
}

void testNoUnknownHarmonicDataIsFabricated()
{
    const auto record =
        buildRecord();

    const auto analysis =
        analyzeCompositionMidiCorpus(
            record);

    const auto sections =
        analyzeCompositionMidiSections(
            record);

    auto harmony =
        analyzeCompositionMidiHarmony(
            record,
            sections);

    // Simulate a valid analysis where harmonic labels are explicitly unknown.
    // The extractor must preserve Unknown instead of fabricating a quality.
    for (auto& harmonicSection :
         harmony.sections)
    {
        harmonicSection.quality =
            ChordQuality::Unknown;
        harmonicSection.valid =
            true;
    }

    const auto motifs =
        analyzeCompositionMidiMotifs(
            record,
            sections);

    CompositionMidiDatasetFeatureExtractor extractor;

    const auto sample =
        extractor.buildSample(
            record,
            analysis,
            sections,
            harmony,
            motifs,
            "real-midi-002");

    expect(
        sample.isValid(),
        "feature sample remains valid");

    const auto harmonyQualityIndex =
        4u;

    for (const auto& section :
         sample.sectionFeatures)
    {
        expect(
            section[harmonyQualityIndex] ==
                1.0,
            "unavailable harmony is explicitly encoded as Unknown");
    }
}

void testTensionInformationPropagates()
{
    const auto record =
        buildRecord();

    const auto analysis =
        analyzeCompositionMidiCorpus(
            record);

    const auto sections =
        analyzeCompositionMidiSections(
            record);

    const auto harmony =
        analyzeCompositionMidiHarmony(
            record,
            sections);

    const auto motifs =
        analyzeCompositionMidiMotifs(
            record,
            sections);

    CompositionMidiDatasetFeatureExtractor extractor;

    const auto sample =
        extractor.buildSample(
            record,
            analysis,
            sections,
            harmony,
            motifs,
            "real-midi-003");

    expect(
        sample.isValid(),
        "tension propagation sample is valid");

    expect(
        sample.sectionFeatures.back()[1] >
            sample.sectionFeatures.front()[1],
        "section tension changes propagate to ML features");
}

void testTransitionFeaturesPropagate()
{
    const auto record =
        buildRecord();

    const auto analysis =
        analyzeCompositionMidiCorpus(
            record);

    const auto sections =
        analyzeCompositionMidiSections(
            record);

    const auto harmony =
        analyzeCompositionMidiHarmony(
            record,
            sections);

    const auto motifs =
        analyzeCompositionMidiMotifs(
            record,
            sections);

    CompositionMidiDatasetFeatureExtractor extractor;

    const auto sample =
        extractor.buildSample(
            record,
            analysis,
            sections,
            harmony,
            motifs,
            "real-midi-004");

    expect(
        sample.isValid(),
        "transition feature sample is valid");

    expect(
        sample.globalFeatures[9] >= 0.0 &&
        sample.globalFeatures[10] >= 0.0 &&
        sample.globalFeatures[11] >= 0.0,
        "transition features are normalized");
}

void testInvalidInputsAreRejected()
{
    CompositionMidiDatasetFeatureExtractor extractor;

    CompositionMidiCorpusRecord invalidRecord;
    CompositionMidiCorpusAnalysis invalidAnalysis;
    CompositionMidiSectionAnalysis invalidSections;

    const auto sample =
        extractor.buildSample(
            invalidRecord,
            invalidAnalysis,
            invalidSections,
            CompositionMidiHarmonyAnalysis{},
            CompositionMidiMotifAnalysis{},
            "invalid");

    expect(
        !sample.isValid(),
        "invalid MIDI inputs are rejected");
}

void testDeterministicExtraction()
{
    const auto record =
        buildRecord();

    const auto analysis =
        analyzeCompositionMidiCorpus(
            record);

    const auto sections =
        analyzeCompositionMidiSections(
            record);

    const auto harmony =
        analyzeCompositionMidiHarmony(
            record,
            sections);

    const auto motifs =
        analyzeCompositionMidiMotifs(
            record,
            sections);

    CompositionMidiDatasetFeatureExtractor extractor;

    const auto first =
        extractor.buildSample(
            record,
            analysis,
            sections,
            harmony,
            motifs,
            "deterministic");

    const auto second =
        extractor.buildSample(
            record,
            analysis,
            sections,
            harmony,
            motifs,
            "deterministic");

    expect(
        first.globalFeatures ==
            second.globalFeatures &&
        first.sectionFeatures ==
            second.sectionFeatures,
        "MIDI dataset extraction is deterministic");
}

} // namespace

int main()
{
    testRealMidiFeaturesBecomeSchemaSample();
    testNoUnknownHarmonicDataIsFabricated();
    testTensionInformationPropagates();
    testTransitionFeaturesPropagate();
    testInvalidInputsAreRejected();
    testDeterministicExtraction();

    std::cout
        << "MIDI-GenGX MIDI dataset feature extractor tests passed.\n";

    return 0;
}
