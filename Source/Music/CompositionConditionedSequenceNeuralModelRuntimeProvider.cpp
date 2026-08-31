#include "CompositionConditionedSequenceNeuralModelRuntimeProvider.h"

#include "MusicalEngine.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace midigengx::music
{
namespace
{
CompositionMidiSequenceWindow makeWindow(
    const std::vector<CompositionMidiTrainingEvent>& context,
    const CompositionSequenceLearningContract& contract) noexcept
{
    CompositionMidiSequenceWindow window;

    if (!contract.isValid() ||
        context.size() != contract.contextLength ||
        contract.inputFeatureWidth !=
            CompositionMidiTrainingEvent::featureCount)
    {
        return window;
    }

    window.contextLength = contract.contextLength;
    window.featureWidth = contract.inputFeatureWidth;
    window.inputs.assign(
        window.contextLength * window.featureWidth,
        0.0);
    window.paddingMask.assign(
        window.contextLength,
        0.0);

    for (std::size_t index = 0;
         index < context.size();
         ++index)
    {
        if (!context[index].isValid())
            return {};

        std::copy(
            context[index].features.begin(),
            context[index].features.end(),
            window.inputs.begin() +
                index * window.featureWidth);

        window.paddingMask[index] = 1.0;
    }

    window.targets = context.back().features;
    window.valid = true;

    if (!window.isValid(contract))
        return {};

    return window;
}

double clampUnit(double value) noexcept
{
    return std::clamp(value, -1.0, 1.0);
}

double normalizedPercent(int value) noexcept
{
    return clampUnit(
        static_cast<double>(value) / 50.0 - 1.0);
}

double normalizedRegister(int value) noexcept
{
    return clampUnit(
        static_cast<double>(value) / 8.0);
}

CompositionMidiTrainingEvent makePrimerEvent(
    const midigengx::domain::MusicalContext& context,
    double progress) noexcept
{
    CompositionMidiTrainingEvent event;
    event.features.assign(
        CompositionMidiTrainingEvent::featureCount,
        0.0);

    event.features[0] =
        clampUnit(
            (static_cast<int>(context.key) + 60) /
                127.0 * 2.0 - 1.0);
    event.features[1] = 0.0;
    event.features[2] = 0.0;
    event.features[3] = 0.0;
    event.features[4] = normalizedRegister(
        context.parameters.octaveLow);
    event.features[5] = normalizedRegister(
        context.parameters.octaveHigh);
    event.features[6] = normalizedPercent(
        context.parameters.density);
    event.features[7] = normalizedPercent(
        context.parameters.variation);
    event.features[8] = normalizedPercent(
        context.parameters.complexity);
    event.features[9] = normalizedPercent(
        context.parameters.syncopation);
    event.features[10] = normalizedPercent(
        context.parameters.tension);
    event.features[11] = normalizedPercent(
        context.parameters.repetition);
    event.features[12] = clampUnit(progress);
    event.features[13] = 0.0;
    event.features[14] = 0.0;
    event.features[15] = 1.0;
    event.features[16] = 0.0;
    event.features[17] = 1.0;
    event.features[18] = clampUnit(progress);
    event.features[19] = 0.0;

    return event;
}

CompositionConditionedTrainingSample makeConditioningSample(
    const CompositionConditionedSequenceNeuralModel& model,
    const std::vector<CompositionMidiTrainingEvent>& primer)
{
    CompositionConditionedTrainingSample sample;

    sample.sequence.sampleId = "runtime-primer";
    sample.sequence.events = primer;
    sample.sequence.featureWidth =
        CompositionMidiTrainingEvent::featureCount;
    sample.sequence.analysisValid = true;

    sample.metadata.sampleId = sample.sequence.sampleId;
    sample.metadata.composerId =
        model.vocabulary.composers.front();
    sample.metadata.workId = "runtime-primer";
    sample.metadata.movementId = "movement-1";
    sample.metadata.styleId =
        model.vocabulary.styles.front();
    sample.metadata.eraId =
        model.vocabulary.eras.front();
    sample.metadata.instrumentationId =
        model.vocabulary.instrumentations.front();
    sample.metadata.verified = true;
    sample.metadata.valid = true;

    sample.composerIndex =
        model.vocabulary.composerIndex(
            sample.metadata.composerId);
    sample.styleIndex =
        model.vocabulary.styleIndex(
            sample.metadata.styleId);
    sample.eraIndex =
        model.vocabulary.eraIndex(
            sample.metadata.eraId);
    sample.instrumentationIndex =
        model.vocabulary.instrumentationIndex(
            sample.metadata.instrumentationId);
    sample.valid = true;

    return sample;
}

} // namespace

bool CompositionConditionedSequenceNeuralModelRuntimeProvider::load(
    const CompositionConditionedSequenceNeuralModelArtifact& artifact) noexcept
{
    CompositionConditionedSequenceNeuralModelRuntimeLoader loader;
    const auto result = loader.load(artifact.bytes);

    if (!result.isValid())
        return false;

    model = result.model;
    ready = model.isValid();
    return ready;
}

bool CompositionConditionedSequenceNeuralModelRuntimeProvider::isReady()
    const noexcept
{
    return ready && model.isValid();
}

void CompositionConditionedSequenceNeuralModelRuntimeProvider::clear() noexcept
{
    model = CompositionConditionedSequenceNeuralModel{};
    ready = false;
}

Phrase CompositionConditionedSequenceNeuralModelRuntimeProvider::generate(
    const midigengx::domain::MusicalContext& context,
    std::uint32_t seed) const noexcept
{
    if (!isReady())
    {
        MusicalEngine engine;
        return engine.generate(context, seed);
    }

    const auto contextLength = model.contract.contextLength;
    if (contextLength == 0 ||
        model.vocabulary.composers.empty() ||
        model.vocabulary.styles.empty() ||
        model.vocabulary.eras.empty() ||
        model.vocabulary.instrumentations.empty())
    {
        MusicalEngine engine;
        return engine.generate(context, seed);
    }

    std::vector<CompositionMidiTrainingEvent> sequence;
    sequence.reserve(contextLength);

    for (std::size_t index = 0;
         index < contextLength;
         ++index)
    {
        const double progress =
            contextLength > 1
                ? static_cast<double>(index) /
                  static_cast<double>(contextLength - 1)
                : 0.0;

        sequence.push_back(
            makePrimerEvent(
                context,
                progress));
    }

    const auto sample =
        makeConditioningSample(
            model,
            sequence);

    if (!sample.isValid())
    {
        MusicalEngine engine;
        return engine.generate(context, seed);
    }

    Phrase result;
    result.lengthBeats =
        static_cast<double>(
            context.parameters.phraseLengthBars) *
        4.0;

    double previousStartBeat = 0.0;
    const std::size_t eventCount =
        std::max<std::size_t>(
            1,
            static_cast<std::size_t>(
                context.parameters.phraseLengthBars) * 4);

    for (std::size_t eventIndex = 0;
         eventIndex < eventCount;
         ++eventIndex)
    {
        const auto window =
            makeWindow(
                sequence,
                model.contract);

        if (!window.isValid(model.contract))
            break;

        const auto prediction =
            model.predictNextEvent(
                window,
                sample);

        if (!prediction.isValid(
                model.contract.targetFeatureWidth))
        {
            break;
        }

        auto decoded =
            CompositionConditionedMidiDecoder::decodeEvent(
                prediction.features,
                sequence.back(),
                previousStartBeat);

        if (decoded.midiNote < 0 ||
            decoded.midiNote > 127 ||
            decoded.velocity < 1 ||
            decoded.velocity > 127 ||
            !std::isfinite(decoded.startBeat) ||
            !std::isfinite(decoded.durationBeats) ||
            decoded.durationBeats <= 0.0)
        {
            break;
        }

        decoded.startBeat =
            std::max(
                decoded.startBeat,
                previousStartBeat);

        result.notes.push_back(decoded);
        previousStartBeat = decoded.startBeat;

        CompositionMidiTrainingEvent generatedEvent;
        generatedEvent.features = prediction.features;
        if (!generatedEvent.isValid())
            break;

        sequence.erase(sequence.begin());
        sequence.push_back(std::move(generatedEvent));
    }

    if (result.notes.empty())
    {
        MusicalEngine engine;
        return engine.generate(context, seed);
    }

    result.lengthBeats =
        previousStartBeat +
        result.notes.back().durationBeats;
    result.normalize();

    MusicalEngine::constrainPhraseToMusicalContext(
        result,
        context);

    return result;
}

} // namespace midigengx::music
