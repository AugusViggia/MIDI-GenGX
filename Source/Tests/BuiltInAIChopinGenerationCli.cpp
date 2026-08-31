#include "Music/BuiltInAIModel.h"
#include "Music/CompositionConditionedSequenceNeuralModelRuntimeProvider.h"
#include "Domain/MusicalContext.h"
#include "Domain/Key.h"
#include "Domain/Scale.h"
#include "Domain/Role.h"
#include "Domain/MusicalParameters.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

using namespace midigengx::music;
using namespace midigengx::domain;

namespace
{
void fail(const char* message)
{
    std::cerr << "FAILED: " << message << '\n';
    std::exit(1);
}

void writeBE16(std::ofstream& file, std::uint16_t value)
{
    const std::uint8_t bytes[] =
    {
        static_cast<std::uint8_t>((value >> 8) & 0xffu),
        static_cast<std::uint8_t>(value & 0xffu)
    };
    file.write(reinterpret_cast<const char*>(bytes), sizeof(bytes));
}

void writeBE32(std::ofstream& file, std::uint32_t value)
{
    const std::uint8_t bytes[] =
    {
        static_cast<std::uint8_t>((value >> 24) & 0xffu),
        static_cast<std::uint8_t>((value >> 16) & 0xffu),
        static_cast<std::uint8_t>((value >> 8) & 0xffu),
        static_cast<std::uint8_t>(value & 0xffu)
    };
    file.write(reinterpret_cast<const char*>(bytes), sizeof(bytes));
}

void writeVariableLength(std::vector<std::uint8_t>& bytes, std::uint32_t value)
{
    std::uint8_t buffer[5]{};
    int count = 1;
    buffer[4] = static_cast<std::uint8_t>(value & 0x7fu);

    while ((value >>= 7u) != 0u)
    {
        ++count;
        buffer[5 - count] = static_cast<std::uint8_t>((value & 0x7fu) | 0x80u);
    }

    bytes.insert(bytes.end(), buffer + (5 - count), buffer + 5);
}

bool writeMidi(const std::filesystem::path& path, const Phrase& phrase)
{
    constexpr std::uint16_t ppq = 480;
    constexpr double beatsPerQuarter = 1.0;

    if (!phrase.isValid() || phrase.notes.empty())
        return false;

    std::vector<std::uint8_t> track;
    std::uint32_t previousTick = 0;

    std::vector<std::pair<std::uint32_t, std::uint8_t>> events;
    events.reserve(phrase.notes.size() * 2);

    for (const auto& note : phrase.notes)
    {
        if (note.midiNote < 0 || note.midiNote > 127 ||
            note.velocity < 1 || note.velocity > 127 ||
            note.startBeat < 0.0 || note.durationBeats <= 0.0)
            return false;

        const auto startTick = static_cast<std::uint32_t>(
            std::max(0.0, note.startBeat * ppq / beatsPerQuarter));
        const auto endTick = static_cast<std::uint32_t>(
            std::max(static_cast<double>(startTick),
                     (note.startBeat + note.durationBeats) * ppq / beatsPerQuarter));

        events.emplace_back(startTick, static_cast<std::uint8_t>(0x90u | ((note.channel - 1) & 0x0fu)));
        events.emplace_back(endTick, static_cast<std::uint8_t>(0x80u | ((note.channel - 1) & 0x0fu)));
    }

    // Regenerate events with pitch/velocity pairs, sorting note-offs before note-ons at equal ticks.
    struct MidiEvent
    {
        std::uint32_t tick;
        std::uint8_t status;
        std::uint8_t data1;
        std::uint8_t data2;
        bool noteOff;
    };

    std::vector<MidiEvent> midiEvents;
    midiEvents.reserve(phrase.notes.size() * 2);

    for (const auto& note : phrase.notes)
    {
        const auto startTick = static_cast<std::uint32_t>(
            std::max(0.0, note.startBeat * ppq));
        const auto endTick = static_cast<std::uint32_t>(
            std::max(static_cast<double>(startTick),
                     (note.startBeat + note.durationBeats) * ppq));
        const auto channel = static_cast<std::uint8_t>((note.channel - 1) & 0x0fu);

        midiEvents.push_back({startTick, static_cast<std::uint8_t>(0x90u | channel),
                              static_cast<std::uint8_t>(note.midiNote),
                              static_cast<std::uint8_t>(note.velocity), false});
        midiEvents.push_back({endTick, static_cast<std::uint8_t>(0x80u | channel),
                              static_cast<std::uint8_t>(note.midiNote), 0, true});
    }

    std::sort(
        midiEvents.begin(), midiEvents.end(),
        [](const MidiEvent& left, const MidiEvent& right)
        {
            if (left.tick != right.tick)
                return left.tick < right.tick;
            if (left.noteOff != right.noteOff)
                return left.noteOff;
            return left.data1 < right.data1;
        });

    for (const auto& event : midiEvents)
    {
        writeVariableLength(track, event.tick - previousTick);
        previousTick = event.tick;
        track.push_back(event.status);
        track.push_back(event.data1);
        track.push_back(event.data2);
    }

    track.push_back(0x00u);
    track.push_back(0xffu);
    track.push_back(0x2fu);
    track.push_back(0x00u);

    std::ofstream file(path, std::ios::binary);
    if (!file)
        return false;

    file.write("MThd", 4);
    writeBE32(file, 6);
    writeBE16(file, 0);
    writeBE16(file, 1);
    writeBE16(file, ppq);
    file.write("MTrk", 4);
    writeBE32(file, static_cast<std::uint32_t>(track.size()));
    file.write(reinterpret_cast<const char*>(track.data()),
               static_cast<std::streamsize>(track.size()));

    return static_cast<bool>(file);
}

MusicalContext makeContext(
    int density,
    int variation,
    int complexity,
    int syncopation,
    int tension,
    int repetition)
{
    MusicalContext context;
    context.key = Key::C;
    context.scale = Scale{ScaleType::Minor};
    context.role = Role::Piano;
    context.parameters.lengthBars = 4;
    context.parameters.phraseLengthBars = 4;
    context.parameters.octaveLow = 1;
    context.parameters.octaveHigh = 4;
    context.parameters.density = density;
    context.parameters.variation = variation;
    context.parameters.complexity = complexity;
    context.parameters.syncopation = syncopation;
    context.parameters.tension = tension;
    context.parameters.repetition = repetition;
    context.normalize();
    return context;
}

bool samePhrase(const Phrase& left, const Phrase& right)
{
    if (left.notes.size() != right.notes.size())
        return false;

    if (left.lengthBeats != right.lengthBeats)
        return false;

    for (std::size_t index = 0; index < left.notes.size(); ++index)
    {
        const auto& a = left.notes[index];
        const auto& b = right.notes[index];
        if (a.midiNote != b.midiNote ||
            a.velocity != b.velocity ||
            a.channel != b.channel ||
            a.startBeat != b.startBeat ||
            a.durationBeats != b.durationBeats)
            return false;
    }

    return true;
}
} // namespace

int main(int argc, char* argv[])
{
    if (argc > 2)
        fail("usage: MIDI_GenGX_BuiltInAIChopinGenerationCli [output-directory]");

    const std::filesystem::path outputDirectory =
        argc == 2 ? std::filesystem::path(argv[1]) : std::filesystem::current_path();

    std::error_code error;
    std::filesystem::create_directories(outputDirectory, error);
    if (error)
        fail("output directory could not be created");

    const auto* data = built_in_ai_model::data();
    const auto size = built_in_ai_model::size();
    if (data == nullptr || size == 0)
        fail("built-in Chopin model resource is empty");

    CompositionConditionedSequenceNeuralModelArtifact artifact;
    artifact.bytes.assign(data, data + size);

    CompositionConditionedSequenceNeuralModelRuntimeProvider provider;
    if (!provider.load(artifact) || !provider.isReady())
        fail("built-in Chopin conditioned model did not become runtime-ready");

    const auto defaultContext = makeContext(50, 25, 50, 50, 50, 50);
    const auto expressiveContext = makeContext(85, 70, 85, 75, 90, 20);

    const auto defaultPhrase = provider.generate(defaultContext, 123);
    const auto expressivePhrase = provider.generate(expressiveContext, 123);

    if (!defaultPhrase.isValid() || defaultPhrase.notes.empty())
        fail("default Chopin AI generation produced an invalid phrase");

    if (!expressivePhrase.isValid() || expressivePhrase.notes.empty())
        fail("expressive Chopin AI generation produced an invalid phrase");

    if (samePhrase(defaultPhrase, expressivePhrase))
        fail("selector-conditioned contexts produced identical output");

    const auto defaultPath = outputDirectory / "ChopinBuiltInAI-default.mid";
    const auto expressivePath = outputDirectory / "ChopinBuiltInAI-expressive.mid";

    if (!writeMidi(defaultPath, defaultPhrase))
        fail("default MIDI output could not be written");

    if (!writeMidi(expressivePath, expressivePhrase))
        fail("expressive MIDI output could not be written");

    std::cout
        << "MIDI-GenGX built-in Chopin AI generation passed.\n"
        << "modelBytes=" << size << "\n"
        << "runtimeReady=true\n"
        << "defaultNotes=" << defaultPhrase.notes.size() << "\n"
        << "expressiveNotes=" << expressivePhrase.notes.size() << "\n"
        << "selectorConditionChangedOutput=true\n"
        << "defaultMidi=" << defaultPath.string() << "\n"
        << "expressiveMidi=" << expressivePath.string() << "\n";

    return 0;
}
