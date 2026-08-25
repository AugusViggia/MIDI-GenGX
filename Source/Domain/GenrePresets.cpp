#include "GenrePresets.h"

namespace midigengx::domain
{
namespace
{
constexpr GenrePresetValues customValues
{
    GenrePreset::Custom,
    50, 25, 50, 50, 50, 50,
    0, 0,
    75,
    NoteLength::Auto,
    PhraseContour::Arch,
    CadenceStyle::Root
};

// These are production-oriented starting points, not universal genre laws.
// There is no authoritative 0-100 standard for Density, Complexity, Tension,
// etc. The profiles below translate documented genre traits into deterministic
// starting values for MIDI-GenGX. Users can immediately refine every value.
constexpr GenrePresetValues houseValues
{
    GenrePreset::House,
    66, 28, 42, 68, 44, 78,
    10, 12,
    80,
    NoteLength::Medium,
    PhraseContour::Arch,
    CadenceStyle::Root
};

constexpr GenrePresetValues deepHouseValues
{
    GenrePreset::DeepHouse,
    52, 24, 34, 74, 36, 84,
    16, 12,
    74,
    NoteLength::Long,
    PhraseContour::Flat,
    CadenceStyle::Root
};

constexpr GenrePresetValues organicHouseValues
{
    GenrePreset::OrganicHouse,
    48, 34, 44, 72, 36, 82,
    24, 20,
    64,
    NoteLength::Long,
    PhraseContour::Arch,
    CadenceStyle::Open
};

constexpr GenrePresetValues progressiveHouseValues
{
    GenrePreset::ProgressiveHouse,
    58, 48, 58, 48, 68, 72,
    8, 18,
    86,
    NoteLength::Long,
    PhraseContour::Arch,
    CadenceStyle::Fifth
};

constexpr GenrePresetValues technoValues
{
    GenrePreset::Techno,
    74, 18, 40, 38, 58, 92,
    4, 6,
    68,
    NoteLength::Short,
    PhraseContour::Flat,
    CadenceStyle::Root
};

constexpr GenrePresetValues tranceValues
{
    GenrePreset::Trance,
    68, 58, 74, 54, 78, 64,
    6, 20,
    92,
    NoteLength::Long,
    PhraseContour::Arch,
    CadenceStyle::Fifth
};

constexpr GenrePresetValues drumAndBassValues
{
    GenrePreset::DrumAndBass,
    82, 64, 72, 86, 60, 58,
    12, 22,
    70,
    NoteLength::Short,
    PhraseContour::Ascending,
    CadenceStyle::Root
};

constexpr GenrePresetValues dubstepValues
{
    GenrePreset::Dubstep,
    56, 70, 84, 82, 88, 44,
    10, 28,
    76,
    NoteLength::Short,
    PhraseContour::Valley,
    CadenceStyle::Root
};

constexpr GenrePresetValues hipHopValues
{
    GenrePreset::HipHop,
    50, 38, 46, 72, 46, 80,
    22, 20,
    76,
    NoteLength::Long,
    PhraseContour::Flat,
    CadenceStyle::Root
};

constexpr GenrePresetValues trapValues
{
    GenrePreset::Trap,
    56, 70, 80, 90, 74, 46,
    12, 26,
    74,
    NoteLength::Short,
    PhraseContour::Valley,
    CadenceStyle::Root
};
}

const GenrePresetValues& getGenrePresetValues(
    GenrePreset genre) noexcept
{
    switch (genre)
    {
        case GenrePreset::House:          return houseValues;
        case GenrePreset::DeepHouse:      return deepHouseValues;
        case GenrePreset::OrganicHouse:    return organicHouseValues;
        case GenrePreset::ProgressiveHouse:return progressiveHouseValues;
        case GenrePreset::Techno:         return technoValues;
        case GenrePreset::Trance:         return tranceValues;
        case GenrePreset::DrumAndBass:    return drumAndBassValues;
        case GenrePreset::Dubstep:        return dubstepValues;
        case GenrePreset::HipHop:         return hipHopValues;
        case GenrePreset::Trap:           return trapValues;
        case GenrePreset::Custom:
        default:                          return customValues;
    }
}

const char* genrePresetName(
    GenrePreset genre) noexcept
{
    switch (genre)
    {
        case GenrePreset::House:           return "House";
        case GenrePreset::DeepHouse:       return "Deep House";
        case GenrePreset::OrganicHouse:     return "Organic House";
        case GenrePreset::ProgressiveHouse:return "Progressive House";
        case GenrePreset::Techno:          return "Techno";
        case GenrePreset::Trance:          return "Trance";
        case GenrePreset::DrumAndBass:     return "Drum & Bass";
        case GenrePreset::Dubstep:         return "Dubstep";
        case GenrePreset::HipHop:          return "Hip-Hop";
        case GenrePreset::Trap:            return "Trap";
        case GenrePreset::Custom:
        default:                            return "Custom";
    }
}
} // namespace midigengx::domain
