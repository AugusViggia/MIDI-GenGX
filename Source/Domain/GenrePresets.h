#pragma once

#include "MusicalParameters.h"

namespace midigengx::domain
{
struct GenrePresetValues
{
    GenrePreset genre = GenrePreset::Custom;

    int density = 50;
    int variation = 25;
    int complexity = 50;
    int syncopation = 50;
    int tension = 50;
    int repetition = 50;

    int humanization = 0;
    int noteLengthVariation = 0;

    int cadenceStrength = 75;

    NoteLength noteLength = NoteLength::Auto;
    PhraseContour phraseContour = PhraseContour::Arch;
    CadenceStyle cadenceStyle = CadenceStyle::Root;
};

const GenrePresetValues& getGenrePresetValues(
    GenrePreset genre) noexcept;

const char* genrePresetName(
    GenrePreset genre) noexcept;

} // namespace midigengx::domain
