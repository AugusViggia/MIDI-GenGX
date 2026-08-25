#pragma once

#include <algorithm>
#include <utility>

namespace midigengx::domain
{

enum class NoteLength
{
    Auto,
    Short,
    Medium,
    Long,
    Legato,
    Staccato,
    Custom
};

enum class RhythmFeel
{
    Straight,
    Syncopated,
    Groovy,
    Driving,
    Offbeat,
    Minimal,
    Custom
};

enum class PhraseContour
{
    Arch,
    Ascending,
    Descending,
    Flat,
    Valley,
    Custom
};

enum class CadenceStyle
{
    Root,
    Fifth,
    Third,
    Open,
    Custom
};

enum class GenrePreset
{
    Custom,
    House,
    DeepHouse,
    OrganicHouse,
    ProgressiveHouse,
    Techno,
    Trance,
    DrumAndBass,
    Dubstep,
    HipHop,
    Trap,
    Count
};

struct MusicalParameters
{
    int octaveLow = -2;
    int octaveHigh = 4;
    // Relative register bias inside the absolute MIDI range.
    int octaveShift = 0;

    int lengthBars = 16;
    int phraseLengthBars = 4;

    int complexity = 50;
    int catchiness = 50;
    int density = 50;
    int syncopation = 50;
    int octaveMovement = 50;
    int variation = 50;
    int repetition = 50;
    int tension = 50;
    int humanization = 0;
    int noteLengthVariation = 0;
    int cadenceStrength = 75;

    GenrePreset genrePreset = GenrePreset::Custom;

    NoteLength noteLength = NoteLength::Auto;
    RhythmFeel rhythm = RhythmFeel::Straight;
    PhraseContour phraseContour = PhraseContour::Arch;
    CadenceStyle cadenceStyle = CadenceStyle::Root;

    void clamp()
    {
        octaveLow = std::clamp(octaveLow, -8, 8);
        octaveHigh = std::clamp(octaveHigh, -8, 8);
        octaveShift = std::clamp(octaveShift, -2, 2);

        if (octaveLow > octaveHigh)
            std::swap(octaveLow, octaveHigh);

        lengthBars = std::clamp(lengthBars, 1, 64);
        phraseLengthBars = std::clamp(
            phraseLengthBars,
            1,
            lengthBars);

        complexity = clampPercent(complexity);
        catchiness = clampPercent(catchiness);
        density = clampPercent(density);
        syncopation = clampPercent(syncopation);
        octaveMovement = clampPercent(octaveMovement);
        variation = clampPercent(variation);
        repetition = clampPercent(repetition);
        tension = clampPercent(tension);
        humanization = clampPercent(humanization);
        noteLengthVariation = clampPercent(noteLengthVariation);
        cadenceStrength = clampPercent(cadenceStrength);
    }

private:
    static int clampPercent(int value)
    {
        return std::clamp(value, 0, 100);
    }
};

} // namespace midigengx::domain
