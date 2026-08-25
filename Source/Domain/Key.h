#pragma once

#include <array>
#include <cstddef>
#include <string_view>

namespace midigengx::domain
{

enum class Key
{
    C = 0,
    CSharp = 1,
    D = 2,
    DSharp = 3,
    E = 4,
    F = 5,
    FSharp = 6,
    G = 7,
    GSharp = 8,
    A = 9,
    ASharp = 10,
    B = 11
};

constexpr int toPitchClass(Key key) noexcept
{
    return static_cast<int>(key);
}

constexpr Key keyFromPitchClass(int pitchClass) noexcept
{
    const auto normalized = ((pitchClass % 12) + 12) % 12;
    return static_cast<Key>(normalized);
}

constexpr std::string_view toString(Key key) noexcept
{
    constexpr std::array<std::string_view, 12> names{
        "C", "C#", "D", "D#", "E", "F",
        "F#", "G", "G#", "A", "A#", "B"
    };

    return names[static_cast<std::size_t>(toPitchClass(key))];
}

} // namespace midigengx::domain
