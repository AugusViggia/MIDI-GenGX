#include "Scale.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace midigengx::domain
{
namespace
{

std::vector<int> normalizeIntervals(std::vector<int> intervals)
{
    for (auto& value : intervals)
        value = ((value % 12) + 12) % 12;

    std::sort(intervals.begin(), intervals.end());

    intervals.erase(
        std::unique(intervals.begin(), intervals.end()),
        intervals.end());

    return intervals;
}

std::vector<int> intervalsFor(ScaleType type)
{
    switch (type)
    {
        case ScaleType::Major:         return {0,2,4,5,7,9,11};
        case ScaleType::Minor:         return {0,2,3,5,7,8,10};
        case ScaleType::HarmonicMinor: return {0,2,3,5,7,8,11};
        case ScaleType::MelodicMinor:  return {0,2,3,5,7,9,11};
        case ScaleType::Dorian:        return {0,2,3,5,7,9,10};
        case ScaleType::Phrygian:      return {0,1,3,5,7,8,10};
        case ScaleType::Lydian:        return {0,2,4,6,7,9,11};
        case ScaleType::Mixolydian:    return {0,2,4,5,7,9,10};
        case ScaleType::Locrian:       return {0,1,3,5,6,8,10};
        case ScaleType::Pentatonic:    return {0,2,4,7,9};
        case ScaleType::Blues:         return {0,3,5,6,7,10};
        case ScaleType::Chromatic:     return {0,1,2,3,4,5,6,7,8,9,10,11};
        case ScaleType::Arabic:        return {0,2,4,5,6,8,10};
        case ScaleType::Rumanian:      return {0,2,3,6,7,9,10};
        case ScaleType::Hindu:         return {0,2,4,5,7,8,10};
        case ScaleType::Spanish:       return {0,1,4,5,7,8,10};
        case ScaleType::Hungarian:     return {0,2,3,6,7,8,11};
        case ScaleType::Count:
            throw std::logic_error("ScaleType::Count is not a concrete scale");
        case ScaleType::Custom:
            throw std::logic_error("ScaleType::Custom requires Scale::custom()");
    }

    throw std::logic_error("Unsupported scale type");
}

const char* scaleName(ScaleType type) noexcept
{
    switch (type)
    {
        case ScaleType::Major: return "Major";
        case ScaleType::Minor: return "Minor";
        case ScaleType::HarmonicMinor: return "Harmonic Minor";
        case ScaleType::MelodicMinor: return "Melodic Minor";
        case ScaleType::Dorian: return "Dorian";
        case ScaleType::Phrygian: return "Phrygian";
        case ScaleType::Lydian: return "Lydian";
        case ScaleType::Mixolydian: return "Mixolydian";
        case ScaleType::Locrian: return "Locrian";
        case ScaleType::Pentatonic: return "Pentatonic";
        case ScaleType::Blues: return "Blues";
        case ScaleType::Chromatic: return "Chromatic";
        case ScaleType::Arabic: return "Arabic";
        case ScaleType::Rumanian: return "Rumanian";
        case ScaleType::Hindu: return "Hindu";
        case ScaleType::Spanish: return "Spanish";
        case ScaleType::Hungarian: return "Hungarian";
        case ScaleType::Count: return "Count";
        case ScaleType::Custom: return "Custom";
    }
    return "Unknown";
}

} // namespace

Scale::Scale() noexcept
    : type_(ScaleType::Major),
      intervals_{0,2,4,5,7,9,11}
{
}

Scale::Scale(ScaleType type)
    : type_(type),
      intervals_(intervalsFor(type))
{
}

Scale Scale::custom(std::vector<int> intervals)
{
    Scale scale;
    scale.type_ = ScaleType::Custom;
    scale.intervals_ = normalizeIntervals(std::move(intervals));
    return scale;
}

ScaleType Scale::getType() const noexcept
{
    return type_;
}

const std::vector<int>& Scale::getIntervals() const noexcept
{
    return intervals_;
}

bool Scale::isValidPitchClass(int pitchClass) const noexcept
{
    const auto normalized = ((pitchClass % 12) + 12) % 12;

    return std::find(
        intervals_.begin(),
        intervals_.end(),
        normalized) != intervals_.end();
}

std::vector<int> Scale::getPitchClasses(int rootPitchClass) const
{
    const auto root = ((rootPitchClass % 12) + 12) % 12;

    std::vector<int> result;
    result.reserve(intervals_.size());

    for (const auto interval : intervals_)
        result.push_back((root + interval) % 12);

    return result;
}

const char* Scale::getName() const noexcept
{
    return scaleName(type_);
}

} // namespace midigengx::domain
