
#include "../Domain/GenrePresets.h"
#include "../Domain/AbletonOctaveConvention.h"
#include "../Domain/Key.h"
#include "../Domain/Scale.h"
#include <iostream>
#include <string>
#include <vector>

namespace { int failures = 0; void expect(bool ok,const char* msg){ if(!ok){++failures; std::cerr<<"FAILED: "<<msg<<"\n";} }
void testKey(){ using namespace midigengx::domain; expect(keyFromPitchClass(12)==Key::C,"12 wraps to C"); expect(keyFromPitchClass(-1)==Key::B,"-1 wraps to B"); expect(toString(Key::FSharp)=="F#","F# name"); }
void testScales()
{
    using namespace midigengx::domain;

    const Scale major(ScaleType::Major);
    const Scale minor(ScaleType::Minor);

    expect(major.isValidPitchClass(0),"C major contains C");
    expect(!major.isValidPitchClass(1),"C major excludes C#");
    expect(minor.isValidPitchClass(3),"C minor contains Eb");
    expect(!minor.isValidPitchClass(4),"C natural minor excludes E");

    const struct
    {
        ScaleType type;
        std::vector<int> expected;
        const char* name;
    } extended[] =
    {
        {ScaleType::Arabic,    {0,2,4,5,6,8,10}, "Arabic"},
        {ScaleType::Rumanian,  {0,2,3,6,7,9,10}, "Rumanian"},
        {ScaleType::Hindu,     {0,2,4,5,7,8,10}, "Hindu"},
        {ScaleType::Spanish,   {0,1,4,5,7,8,10}, "Spanish"},
        {ScaleType::Hungarian, {0,2,3,6,7,8,11}, "Hungarian"}
    };

    for (const auto& item : extended)
    {
        const Scale scale(item.type);

        expect(
            std::string(scale.getName()) == item.name,
            "extended scale name is stable");

        expect(
            scale.getIntervals() == item.expected,
            "extended scale interval formula is correct");

        expect(
            scale.getPitchClasses(0) == item.expected,
            "extended scale pitch classes are correct");
    }
}

void testAbletonOctaveConvention()
{
    using Convention =
        midigengx::domain::AbletonOctaveConvention;

    expect(
        Convention::abletonOctaveToInternal(2) == 3,
        "Ableton C2 maps to internal register 3");

    expect(
        Convention::abletonOctaveToInternal(4) == 5,
        "Ableton C4 maps to internal register 5");

    expect(
        Convention::internalToAbletonOctave(3) == 2,
        "internal register 3 displays as Ableton C2");

    expect(
        Convention::internalToAbletonOctave(5) == 4,
        "internal register 5 displays as Ableton C4");

    expect(
        Convention::midiForC(2) == 48,
        "Ableton C2 is MIDI 48");

    expect(
        Convention::midiForC(4) == 72,
        "Ableton C4 is MIDI 72");

    expect(
        Convention::midiForC(
            Convention::internalToAbletonOctave(3)) == 48,
        "internal register round-trips to MIDI C2");

    expect(
        Convention::minAbletonOctave == -2 &&
        Convention::maxAbletonOctave == 8,
        "Ableton octave UI range is -2 through 8");

    expect(
        Convention::abletonOctaveToInternal(8) == 9,
        "Ableton C8 maps to internal register 9");

    expect(
        Convention::internalToAbletonOctave(9) == 8,
        "internal register 9 displays as Ableton octave 8");

    expect(
        Convention::midiForC(8) == 120,
        "Ableton C8 is MIDI 120");
}

void testGenreProfiles()
{
    using namespace midigengx::domain;

    const auto& house =
        getGenrePresetValues(GenrePreset::House);

    const auto& techno =
        getGenrePresetValues(GenrePreset::Techno);

    const auto& trance =
        getGenrePresetValues(GenrePreset::Trance);

    expect(
        house.genre == GenrePreset::House,
        "House identity");

    expect(
        techno.genre == GenrePreset::Techno,
        "Techno identity");

    expect(
        trance.genre == GenrePreset::Trance,
        "Trance identity");

    expect(
        house.density != techno.density ||
            house.syncopation != techno.syncopation ||
            house.repetition != techno.repetition,
        "House and Techno differ");

    expect(
        trance.complexity != techno.complexity ||
            trance.tension != techno.tension ||
            trance.cadenceStrength != techno.cadenceStrength,
        "Trance and Techno differ");
}

void testAllGenreProfiles()
{
    using namespace midigengx::domain;

    const GenrePreset genres[] =
    {
        GenrePreset::House,
        GenrePreset::DeepHouse,
        GenrePreset::OrganicHouse,
        GenrePreset::ProgressiveHouse,
        GenrePreset::Techno,
        GenrePreset::Trance,
        GenrePreset::DrumAndBass,
        GenrePreset::Dubstep,
        GenrePreset::HipHop,
        GenrePreset::Trap
    };

    for (const auto genre : genres)
    {
        const auto& profile =
            getGenrePresetValues(genre);

        expect(profile.genre == genre,
               "Genre profile identity is stable");

        expect(profile.density >= 0 && profile.density <= 100,
               "Genre density is bounded");

        expect(profile.variation >= 0 && profile.variation <= 100,
               "Genre variation is bounded");

        expect(profile.complexity >= 0 && profile.complexity <= 100,
               "Genre complexity is bounded");

        expect(profile.syncopation >= 0 && profile.syncopation <= 100,
               "Genre syncopation is bounded");

        expect(profile.tension >= 0 && profile.tension <= 100,
               "Genre tension is bounded");

        expect(profile.repetition >= 0 && profile.repetition <= 100,
               "Genre repetition is bounded");

        expect(profile.humanization >= 0 && profile.humanization <= 100,
               "Genre humanization is bounded");

        expect(profile.noteLengthVariation >= 0 &&
                   profile.noteLengthVariation <= 100,
               "Genre length variation is bounded");

        expect(profile.cadenceStrength >= 0 &&
                   profile.cadenceStrength <= 100,
               "Genre cadence strength is bounded");
    }
}


} // namespace

int main()
{
    testKey();
    testScales();
    testAbletonOctaveConvention();
    testGenreProfiles();
    testAllGenreProfiles();

    if (failures != 0)
        return 1;

    std::cout
        << "MIDI-GenGX Domain Layer tests passed.\n";

    return 0;
}
