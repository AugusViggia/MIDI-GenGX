
#pragma once

namespace midigengx::domain
{

struct AbletonOctaveConvention
{
    // MIDI-GenGX keeps a one-step internal register offset so existing
    // serialized processor state remains compatible. The UI exposes the
    // user-facing Ableton octave numbers directly: -2 through 8.
    //
    // Therefore:
    //   Ableton octave -2 -> internal register -1
    //   Ableton octave  8 -> internal register  9
    //
    // The musical engine converts the internal register to the corresponding
    // absolute C boundary before filtering by the selected key/scale.

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
