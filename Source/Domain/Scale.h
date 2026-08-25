#pragma once

#include <vector>

namespace midigengx::domain
{

enum class ScaleType
{
    Major,
    Minor,
    HarmonicMinor,
    MelodicMinor,
    Dorian,
    Phrygian,
    Lydian,
    Mixolydian,
    Locrian,
    Pentatonic,
    Blues,
    Chromatic,
    Arabic,
    Rumanian,
    Hindu,
    Spanish,
    Hungarian,
    Count,
    Custom
};

class Scale
{
public:
    Scale() noexcept;
    explicit Scale(ScaleType type);

    static Scale custom(std::vector<int> intervals);

    ScaleType getType() const noexcept;
    const std::vector<int>& getIntervals() const noexcept;

    bool isValidPitchClass(int pitchClass) const noexcept;
    std::vector<int> getPitchClasses(int rootPitchClass) const;
    const char* getName() const noexcept;

private:
    ScaleType type_;
    std::vector<int> intervals_;
};

} // namespace midigengx::domain
