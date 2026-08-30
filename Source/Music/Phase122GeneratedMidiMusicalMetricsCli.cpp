
#include "Music/CompositionMidiCorpusDirectoryLoader.h"
#include "Music/CompositionMidiCorpusRecord.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <vector>

using namespace midigengx::music;

namespace
{
struct Metrics
{
    std::size_t noteCount = 0;
    std::size_t uniquePitches = 0;
    int minPitch = 0;
    int maxPitch = 0;
    double pitchMean = 0.0;
    double pitchStdDev = 0.0;
    double pitchEntropyBits = 0.0;
    double meanAbsInterval = 0.0;
    double intervalStdDev = 0.0;
    double repeatedPitchRatio = 0.0;
    double meanOnsetDeltaBeats = 0.0;
    double onsetDeltaStdDev = 0.0;
    double meanDurationBeats = 0.0;
    double durationStdDev = 0.0;
    double meanVelocity = 0.0;
    double velocityStdDev = 0.0;
    double densityNotesPerBeat = 0.0;
    int maximumPolyphony = 0;
    double lengthBeats = 0.0;
};

double mean(const std::vector<double>& values)
{
    if (values.empty())
        return 0.0;

    return std::accumulate(
        values.begin(), values.end(), 0.0) /
        static_cast<double>(values.size());
}

double stddev(
    const std::vector<double>& values,
    double m)
{
    if (values.empty())
        return 0.0;

    double sum = 0.0;

    for (const double value : values)
    {
        const double delta = value - m;
        sum += delta * delta;
    }

    return std::sqrt(
        sum / static_cast<double>(values.size()));
}

double entropy(
    const std::map<int, std::size_t>& counts)
{
    std::size_t total = 0;

    for (const auto& entry : counts)
        total += entry.second;

    if (total == 0)
        return 0.0;

    double result = 0.0;

    for (const auto& entry : counts)
    {
        if (entry.second == 0)
            continue;

        const double p =
            static_cast<double>(entry.second) /
            static_cast<double>(total);

        result -= p * std::log2(p);
    }

    return result;
}

bool evaluate(
    const CompositionMidiCorpusRecord& record,
    Metrics& result)
{
    if (!record.isValid() ||
        record.notes.empty() ||
        record.ticksPerQuarterNote == 0)
    {
        return false;
    }

    std::vector<double> pitches;
    std::vector<double> intervals;
    std::vector<double> absIntervals;
    std::vector<double> onsetDeltas;
    std::vector<double> durations;
    std::vector<double> velocities;
    std::map<int, std::size_t> pitchCounts;

    for (const auto& note : record.notes)
    {
        pitches.push_back(
            static_cast<double>(note.midiNote));

        pitchCounts[
            static_cast<int>(note.midiNote)] += 1;

        durations.push_back(
            static_cast<double>(
                note.endTick - note.startTick) /
            static_cast<double>(
                record.ticksPerQuarterNote));

        velocities.push_back(
            static_cast<double>(note.velocity));
    }

    for (std::size_t i = 1; i < record.notes.size(); ++i)
    {
        const double interval =
            static_cast<double>(
                static_cast<int>(record.notes[i].midiNote) -
                static_cast<int>(record.notes[i - 1].midiNote));

        intervals.push_back(interval);
        absIntervals.push_back(std::abs(interval));

        onsetDeltas.push_back(
            static_cast<double>(
                record.notes[i].startTick -
                record.notes[i - 1].startTick) /
            static_cast<double>(
                record.ticksPerQuarterNote));
    }

    const double pitchMean = mean(pitches);
    const double intervalMean = mean(intervals);
    const double onsetMean = mean(onsetDeltas);
    const double durationMean = mean(durations);
    const double velocityMean = mean(velocities);

    result.noteCount = record.notes.size();
    result.uniquePitches = pitchCounts.size();
    result.minPitch = static_cast<int>(
        *std::min_element(pitches.begin(), pitches.end()));
    result.maxPitch = static_cast<int>(
        *std::max_element(pitches.begin(), pitches.end()));
    result.pitchMean = pitchMean;
    result.pitchStdDev = stddev(pitches, pitchMean);
    result.pitchEntropyBits = entropy(pitchCounts);
    result.meanAbsInterval = mean(absIntervals);
    result.intervalStdDev = stddev(intervals, intervalMean);

    if (!intervals.empty())
    {
        std::size_t repeated = 0;

        for (const double interval : intervals)
        {
            if (interval == 0.0)
                ++repeated;
        }

        result.repeatedPitchRatio =
            static_cast<double>(repeated) /
            static_cast<double>(intervals.size());
    }

    result.meanOnsetDeltaBeats = onsetMean;
    result.onsetDeltaStdDev = stddev(onsetDeltas, onsetMean);
    result.meanDurationBeats = durationMean;
    result.durationStdDev = stddev(durations, durationMean);
    result.meanVelocity = velocityMean;
    result.velocityStdDev = stddev(velocities, velocityMean);

    result.lengthBeats =
        static_cast<double>(record.lengthTicks) /
        static_cast<double>(record.ticksPerQuarterNote);

    result.densityNotesPerBeat =
        result.lengthBeats > 0.0
            ? static_cast<double>(result.noteCount) /
              result.lengthBeats
            : 0.0;

    std::vector<std::pair<std::uint32_t, int>> boundaries;

    for (const auto& note : record.notes)
    {
        boundaries.emplace_back(note.startTick, +1);
        boundaries.emplace_back(note.endTick, -1);
    }

    std::sort(
        boundaries.begin(),
        boundaries.end(),
        [](const auto& left, const auto& right)
        {
            if (left.first != right.first)
                return left.first < right.first;

            return left.second < right.second;
        });

    int active = 0;

    for (const auto& boundary : boundaries)
    {
        active =
            boundary.second < 0
                ? std::max(0, active - 1)
                : active + 1;

        result.maximumPolyphony =
            std::max(result.maximumPolyphony, active);
    }

    return true;
}

bool loadOne(
    const std::string& path,
    CompositionMidiCorpusRecord& record)
{
    const std::filesystem::path input(path);

    std::error_code error;
    const auto tempRoot =
        std::filesystem::temp_directory_path(error) /
        "MIDI_GenGX_Phase122_Metrics" /
        input.stem();

    if (error)
        return false;

    std::filesystem::remove_all(
        tempRoot,
        error);

    std::filesystem::create_directories(
        tempRoot,
        error);

    if (error)
        return false;

    const auto isolatedFile =
        tempRoot /
        input.filename();

    std::filesystem::copy_file(
        input,
        isolatedFile,
        std::filesystem::copy_options::overwrite_existing,
        error);

    if (error)
    {
        std::filesystem::remove_all(
            tempRoot,
            error);
        return false;
    }

    const auto loaded =
        loadCompositionMidiCorpusDirectory(
            tempRoot.string(),
            false);

    std::filesystem::remove_all(
        tempRoot,
        error);

    if (!loaded.isValid() ||
        loaded.records.size() != 1)
    {
        return false;
    }

    record = loaded.records.front();
    return true;
}

void dump(
    const std::string& label,
    const Metrics& m,
    std::ostream& out)
{
    out
        << label << ".noteCount=" << m.noteCount << '\n'
        << label << ".uniquePitches=" << m.uniquePitches << '\n'
        << label << ".minPitch=" << m.minPitch << '\n'
        << label << ".maxPitch=" << m.maxPitch << '\n'
        << label << ".pitchMean=" << std::setprecision(12) << m.pitchMean << '\n'
        << label << ".pitchStdDev=" << m.pitchStdDev << '\n'
        << label << ".pitchEntropyBits=" << m.pitchEntropyBits << '\n'
        << label << ".meanAbsInterval=" << m.meanAbsInterval << '\n'
        << label << ".intervalStdDev=" << m.intervalStdDev << '\n'
        << label << ".repeatedPitchRatio=" << m.repeatedPitchRatio << '\n'
        << label << ".meanOnsetDeltaBeats=" << m.meanOnsetDeltaBeats << '\n'
        << label << ".onsetDeltaStdDev=" << m.onsetDeltaStdDev << '\n'
        << label << ".meanDurationBeats=" << m.meanDurationBeats << '\n'
        << label << ".durationStdDev=" << m.durationStdDev << '\n'
        << label << ".meanVelocity=" << m.meanVelocity << '\n'
        << label << ".velocityStdDev=" << m.velocityStdDev << '\n'
        << label << ".densityNotesPerBeat=" << m.densityNotesPerBeat << '\n'
        << label << ".maximumPolyphony=" << m.maximumPolyphony << '\n'
        << label << ".lengthBeats=" << m.lengthBeats << '\n';
}
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr
            << "FAILED: usage: "
            << "Phase122GeneratedMidiMusicalMetrics "
            << "<phase119.mid> <phase121.mid>\n";
        return 1;
    }

    CompositionMidiCorpusRecord phase119;
    CompositionMidiCorpusRecord phase121;

    if (!loadOne(argv[1], phase119) ||
        !loadOne(argv[2], phase121))
    {
        std::cerr
            << "FAILED: could not load generated MIDI artifacts\n";
        return 1;
    }

    Metrics baseline;
    Metrics finalModel;

    if (!evaluate(phase119, baseline) ||
        !evaluate(phase121, finalModel))
    {
        std::cerr
            << "FAILED: invalid generated MIDI artifact\n";
        return 1;
    }

    std::ofstream report(
        "phase122-generated-midi-musical-metrics.txt");

    if (!report)
    {
        std::cerr
            << "FAILED: could not create project-root report\n";
        return 1;
    }

    report
        << "PHASE 122 GENERATED MIDI MUSICAL METRICS\n"
        << "phase119Input=" << argv[1] << '\n'
        << "phase121Input=" << argv[2] << '\n';

    dump("phase119", baseline, report);
    dump("phase121", finalModel, report);

    report
        << "analysisComplete=1\n";

    report.flush();

    std::ifstream display(
        "phase122-generated-midi-musical-metrics.txt");

    std::cout << display.rdbuf();
    return 0;
}
