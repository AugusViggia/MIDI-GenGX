#include "Music/CompositionConditionedSequenceNeuralModelRuntimeLoader.h"
#include "Music/CompositionRealComposerCorpusPreparation.h"
#include "Music/CompositionSequenceMetadataFileLoader.h"
#include "Music/CompositionConditionedMidiDecoder.h"
#include "Music/CompositionMidiSequenceWindow.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <string>
#include <vector>

using namespace midigengx::music;

namespace
{
std::vector<std::uint8_t> readBinary(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) return {};
    return std::vector<std::uint8_t>(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>());
}

bool loadModel(
    const std::string& path,
    CompositionConditionedSequenceNeuralModel& model)
{
    const auto bytes = readBinary(path);
    if (bytes.empty()) return false;

    const auto loaded =
        CompositionConditionedSequenceNeuralModelRuntimeLoader{}.load(bytes);

    if (!loaded.isValid()) return false;
    model = loaded.model;
    return true;
}

CompositionMidiSequenceWindow makeWindow(
    const std::vector<CompositionMidiTrainingEvent>& context,
    const CompositionSequenceLearningContract& contract)
{
    CompositionMidiSequenceWindow window;

    if (!contract.isValid() ||
        context.size() != contract.contextLength)
        return window;

    window.contextLength = contract.contextLength;
    window.featureWidth = contract.inputFeatureWidth;
    window.inputs.assign(
        contract.contextLength * contract.inputFeatureWidth,
        0.0);
    window.paddingMask.assign(
        contract.contextLength,
        1.0);

    for (std::size_t index = 0; index < context.size(); ++index)
    {
        if (!context[index].isValid()) return {};
        std::copy(
            context[index].features.begin(),
            context[index].features.end(),
            window.inputs.begin() +
                index * contract.inputFeatureWidth);
    }

    window.targets = context.back().features;
    window.valid = true;

    return window.isValid(contract)
        ? window
        : CompositionMidiSequenceWindow{};
}

struct Stats
{
    std::vector<double> minValue;
    std::vector<double> maxValue;
    std::vector<double> sum;
    std::vector<std::size_t> noteHistogram =
        std::vector<std::size_t>(128, 0);
    std::size_t count = 0;
    std::size_t decoded = 0;
};

bool run(
    const std::string& label,
    const CompositionConditionedSequenceNeuralModel& model,
    const CompositionConditionedTrainingSample& sample,
    std::size_t count,
    Stats& stats,
    std::ostream& out)
{
    const auto featureCount =
        model.contract.targetFeatureWidth;

    if (!model.isValid() ||
        !sample.isValid() ||
        sample.sequence.events.size() <=
            model.contract.contextLength)
        return false;

    stats.minValue.assign(
        featureCount,
        std::numeric_limits<double>::infinity());
    stats.maxValue.assign(
        featureCount,
        -std::numeric_limits<double>::infinity());
    stats.sum.assign(featureCount, 0.0);

    const auto limit =
        std::min(
            count,
            sample.sequence.events.size() -
                model.contract.contextLength);

    for (std::size_t offset = 0; offset < limit; ++offset)
    {
        const std::size_t targetIndex =
            model.contract.contextLength + offset;

        std::vector<CompositionMidiTrainingEvent> context;
        context.reserve(model.contract.contextLength);

        const std::size_t first =
            targetIndex -
            model.contract.contextLength;

        for (std::size_t i = first; i < targetIndex; ++i)
            context.push_back(sample.sequence.events[i]);

        const auto window =
            makeWindow(context, model.contract);

        if (!window.isValid(model.contract))
            return false;

        const auto prediction =
            model.predictNextEvent(window, sample);

        if (!prediction.isValid(
                model.contract.targetFeatureWidth))
            return false;

        ++stats.count;

        for (std::size_t feature = 0;
             feature < featureCount;
             ++feature)
        {
            stats.minValue[feature] =
                std::min(
                    stats.minValue[feature],
                    prediction.features[feature]);

            stats.maxValue[feature] =
                std::max(
                    stats.maxValue[feature],
                    prediction.features[feature]);

            stats.sum[feature] +=
                prediction.features[feature];
        }

        const auto decoded =
            CompositionConditionedMidiDecoder::decodeEvent(
                prediction.features,
                sample.sequence.events[targetIndex - 1],
                0.0,
                CompositionConditionedMidiDecoderConfig{});

        if (decoded.midiNote >= 0 &&
            decoded.midiNote <= 127 &&
            decoded.velocity >= 1 &&
            decoded.velocity <= 127 &&
            decoded.durationBeats > 0.0 &&
            std::isfinite(decoded.durationBeats))
        {
            ++stats.decoded;
            ++stats.noteHistogram[
                static_cast<std::size_t>(decoded.midiNote)];
        }
    }

    std::size_t unique = 0;
    for (const auto value : stats.noteHistogram)
        if (value > 0) ++unique;

    out
        << label << ".predictionCount=" << stats.count << '\n'
        << label << ".decodedCount=" << stats.decoded << '\n'
        << label << ".uniqueDecodedNotes=" << unique << '\n';

    out << label << ".decodedNotes=";
    bool firstNote = true;

    for (std::size_t note = 0;
         note < stats.noteHistogram.size();
         ++note)
    {
        if (stats.noteHistogram[note] == 0) continue;
        if (!firstNote) out << ',';
        firstNote = false;
        out << note << ':' << stats.noteHistogram[note];
    }
    out << '\n';

    for (std::size_t feature = 0;
         feature < featureCount;
         ++feature)
    {
        const double mean =
            stats.count == 0
                ? 0.0
                : stats.sum[feature] /
                    static_cast<double>(stats.count);

        out
            << label << ".feature" << feature
            << ".min=" << std::setprecision(9)
            << stats.minValue[feature] << '\n'
            << label << ".feature" << feature
            << ".max=" << std::setprecision(9)
            << stats.maxValue[feature] << '\n'
            << label << ".feature" << feature
            << ".mean=" << std::setprecision(9)
            << mean << '\n';
    }

    return true;
}
}

int main(int argc, char* argv[])
{
    if (argc != 5)
    {
        std::cerr
            << "FAILED: usage: Phase121GenerativeCollapseDiagnostic "
            << "<corpus> <metadata.tsv> <phase119.mgcn> <phase121.mgcn>\n";
        return 1;
    }

    const auto metadata =
        loadCompositionSequenceMetadataFile(argv[2]);

    if (!metadata.isValid())
    {
        std::cerr << "FAILED: metadata invalid\n";
        return 1;
    }

    const auto prepared =
        prepareRealComposerCorpusFromDirectory(
            argv[1], true, metadata.catalog);

    if (!prepared.isValid() ||
        prepared.conditionedDataset.sampleCount() != 67)
    {
        std::cerr << "FAILED: conditioned corpus invalid\n";
        return 1;
    }

    const auto& sample =
        prepared.conditionedDataset.samples.front();

    CompositionConditionedSequenceNeuralModel phase119;
    CompositionConditionedSequenceNeuralModel phase121;

    if (!loadModel(argv[3], phase119) ||
        !loadModel(argv[4], phase121))
    {
        std::cerr << "FAILED: model load failed\n";
        return 1;
    }

    std::ofstream report(
        "phase121-generative-collapse-diagnostic.txt");

    if (!report)
    {
        std::cerr
            << "FAILED: cannot create project-root report\n";
        return 1;
    }

    report
        << "PHASE 121 GENERATIVE COLLAPSE DIAGNOSTIC\n"
        << "seedSampleId=" << sample.sequence.sampleId << '\n'
        << "teacherForcedPredictionCount=128\n";

    Stats baseline;
    Stats finalModel;

    if (!run(
            "phase119",
            phase119,
            sample,
            128,
            baseline,
            report) ||
        !run(
            "phase121",
            phase121,
            sample,
            128,
            finalModel,
            report))
    {
        std::cerr
            << "FAILED: diagnostic execution failed\n";
        return 1;
    }

    report
        << "diagnosticComplete=1\n";

    report.flush();

    std::cout
        << "PHASE 121 GENERATIVE COLLAPSE DIAGNOSTIC COMPLETE\n"
        << "phase119.predictions=" << baseline.count << '\n'
        << "phase119.decoded=" << baseline.decoded << '\n'
        << "phase121.predictions=" << finalModel.count << '\n'
        << "phase121.decoded=" << finalModel.decoded << '\n'
        << "report=phase121-generative-collapse-diagnostic.txt\n";

    return 0;
}
