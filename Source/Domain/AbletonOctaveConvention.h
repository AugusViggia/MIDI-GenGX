
#pragma once

namespace midigengx::domain
{

struct AbletonOctaveConvention
{
    // MIDI-GenGX keeps an internal C-register offset where:
    // internal 2 -> MIDI C2 in mathematical naming / C1 in Ableton naming.
    //
    // The UI exposes Ableton-compatible octave labels, while the engine keeps
    // its existing internal register representation stable.

    static constexpr int minInternalRegister = -1;
    static constexpr int maxInternalRegister = 9;

    static constexpr int minAbletonOctave = -2;
    static constexpr int maxAbletonOctave = 8;

    static constexpr int abletonOctaveToInternal(
        int abletonOctave) noexcept
    {
        return abletonOctave + 1;
    }

    static constexpr int internalToAbletonOctave(
        int internalRegister) noexcept
    {
        return internalRegister - 1;
    }

    static constexpr int midiForC(
        int abletonOctave) noexcept
    {
        return 12 * (abletonOctave + 2);
    }
};

} // namespace midigengx::domain

