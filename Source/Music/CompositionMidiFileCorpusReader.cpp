#include "CompositionMidiFileCorpusReader.h"

#include <algorithm>
#include <array>
#include <limits>
#include <utility>

namespace midigengx::music
{
namespace
{

std::uint16_t readU16BE(
    const std::uint8_t* data) noexcept
{
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(data[0]) << 8) |
        static_cast<std::uint16_t>(data[1]));
}

std::uint32_t readU32BE(
    const std::uint8_t* data) noexcept
{
    return
        (static_cast<std::uint32_t>(data[0]) << 24) |
        (static_cast<std::uint32_t>(data[1]) << 16) |
        (static_cast<std::uint32_t>(data[2]) << 8) |
        static_cast<std::uint32_t>(data[3]);
}

bool readVlq(
    const std::uint8_t* data,
    std::size_t size,
    std::size_t& cursor,
    std::uint32_t& value) noexcept
{
    value = 0;

    for (int count = 0;
         count < 4;
         ++count)
    {
        if (cursor >= size)
            return false;

        const auto byte =
            data[cursor++];

        if (value >
            (std::numeric_limits<std::uint32_t>::max() >>
             7))
        {
            return false;
        }

        value =
            (value << 7) |
            static_cast<std::uint32_t>(
                byte & 0x7Fu);

        if ((byte & 0x80u) == 0)
            return true;
    }

    return false;
}

struct PendingNote
{
    std::uint32_t startTick = 0;
    std::uint8_t velocity = 0;
    bool active = false;
};

bool hasBytes(
    std::size_t cursor,
    std::size_t amount,
    std::size_t size) noexcept
{
    return cursor <= size &&
           amount <=
               size - cursor;
}

CompositionMidiCorpusRecord fail(
    const std::string& sampleId) noexcept
{
    CompositionMidiCorpusRecord result;
    result.sampleId = sampleId;
    return result;
}

} // namespace

CompositionMidiCorpusRecord
CompositionMidiFileCorpusReader::read(
    const std::string& sampleId,
    const std::uint8_t* data,
    std::size_t size) const noexcept
{
    if (data == nullptr ||
        size < 14 ||
        sampleId.empty())
    {
        return fail(sampleId);
    }

    if (data[0] != 'M' ||
        data[1] != 'T' ||
        data[2] != 'h' ||
        data[3] != 'd')
    {
        return fail(sampleId);
    }

    const auto headerLength =
        readU32BE(data + 4);

    if (headerLength != 6 ||
        !hasBytes(8, headerLength, size))
    {
        return fail(sampleId);
    }

    const auto format =
        readU16BE(data + 8);

    const auto trackCount =
        readU16BE(data + 10);

    const auto division =
        readU16BE(data + 12);

    if ((format != 0 && format != 1) ||
        trackCount == 0 ||
        (division & 0x8000u) != 0 ||
        division == 0)
    {
        return fail(sampleId);
    }

    CompositionMidiCorpusRecord result;
    result.sampleId = sampleId;
    result.ticksPerQuarterNote = division;
    result.trackCount = trackCount;

    std::size_t cursor = 14;
    std::uint32_t maximumEndTick = 0;

    for (std::uint16_t trackIndex = 0;
         trackIndex < trackCount;
         ++trackIndex)
    {
        if (!hasBytes(cursor, 8, size) ||
            data[cursor] != 'M' ||
            data[cursor + 1] != 'T' ||
            data[cursor + 2] != 'r' ||
            data[cursor + 3] != 'k')
        {
            return fail(sampleId);
        }

        const auto trackLength =
            readU32BE(data + cursor + 4);

        cursor += 8;

        if (!hasBytes(
                cursor,
                trackLength,
                size))
        {
            return fail(sampleId);
        }

        const auto trackEnd =
            cursor + trackLength;

        std::array<
            std::array<
                PendingNote,
                128>,
            16> pending{};

        std::uint32_t tick = 0;
        std::uint8_t runningStatus = 0;

        while (cursor < trackEnd)
        {
            std::uint32_t delta = 0;

            if (!readVlq(
                    data,
                    trackEnd,
                    cursor,
                    delta))
            {
                return fail(sampleId);
            }

            if (tick >
                std::numeric_limits<std::uint32_t>::max() -
                    delta)
            {
                return fail(sampleId);
            }

            tick += delta;

            if (cursor >= trackEnd)
                return fail(sampleId);

            auto status =
                data[cursor++];

            if (status < 0x80u)
            {
                if (runningStatus < 0x80u)
                    return fail(sampleId);

                --cursor;
                status = runningStatus;
            }
            else if (status < 0xF0u)
            {
                runningStatus = status;
            }

            if (status == 0xFFu)
            {
                if (cursor >= trackEnd)
                    return fail(sampleId);

                ++cursor;

                std::uint32_t metaLength = 0;

                if (!readVlq(
                        data,
                        trackEnd,
                        cursor,
                        metaLength) ||
                    !hasBytes(
                        cursor,
                        metaLength,
                        trackEnd))
                {
                    return fail(sampleId);
                }

                cursor += metaLength;
                continue;
            }

            if (status == 0xF0u ||
                status == 0xF7u)
            {
                std::uint32_t sysexLength = 0;

                if (!readVlq(
                        data,
                        trackEnd,
                        cursor,
                        sysexLength) ||
                    !hasBytes(
                        cursor,
                        sysexLength,
                        trackEnd))
                {
                    return fail(sampleId);
                }

                cursor += sysexLength;
                continue;
            }

            if (status < 0x80u)
                return fail(sampleId);

            const auto messageType =
                status & 0xF0u;

            const auto channel =
                status & 0x0Fu;

            const auto dataBytes =
                (messageType == 0xC0u ||
                 messageType == 0xD0u)
                    ? 1u
                    : 2u;

            if (!hasBytes(
                    cursor,
                    dataBytes,
                    trackEnd))
            {
                return fail(sampleId);
            }

            const auto first =
                data[cursor++];

            const auto second =
                dataBytes == 2
                    ? data[cursor++]
                    : 0;

            if (messageType == 0x90u)
            {
                if (first > 127)
                    return fail(sampleId);

                auto& pendingNote =
                    pending[channel][first];

                if (second == 0)
                {
                    if (pendingNote.active)
                    {
                        CompositionMidiNote note;

                        note.channel = channel;
                        note.midiNote = first;
                        note.velocity =
                            pendingNote.velocity;
                        note.startTick =
                            pendingNote.startTick;
                        note.endTick =
                            tick;

                        if (note.isValid())
                        {
                            result.notes.push_back(
                                note);

                            maximumEndTick =
                                std::max(
                                    maximumEndTick,
                                    note.endTick);
                        }

                        pendingNote.active = false;
                    }
                }
                else
                {
                    if (pendingNote.active)
                    {
                        CompositionMidiNote note;

                        note.channel = channel;
                        note.midiNote = first;
                        note.velocity =
                            pendingNote.velocity;
                        note.startTick =
                            pendingNote.startTick;
                        note.endTick =
                            tick;

                        if (note.isValid())
                        {
                            result.notes.push_back(
                                note);

                            maximumEndTick =
                                std::max(
                                    maximumEndTick,
                                    note.endTick);
                        }
                    }

                    pendingNote.startTick =
                        tick;

                    pendingNote.velocity =
                        second;

                    pendingNote.active =
                        true;
                }
            }
            else if (messageType == 0x80u)
            {
                if (first > 127)
                    return fail(sampleId);

                auto& pendingNote =
                    pending[channel][first];

                if (pendingNote.active)
                {
                    CompositionMidiNote note;

                    note.channel = channel;
                    note.midiNote = first;
                    note.velocity =
                        pendingNote.velocity;
                    note.startTick =
                        pendingNote.startTick;
                    note.endTick =
                        tick;

                    if (note.isValid())
                    {
                        result.notes.push_back(
                            note);

                        maximumEndTick =
                            std::max(
                                maximumEndTick,
                                note.endTick);
                    }

                    pendingNote.active = false;
                }
            }
        }

        cursor =
            trackEnd;

        if (!result.notes.empty())
        {
            result.lengthTicks =
                std::max(
                    result.lengthTicks,
                    maximumEndTick);
        }
    }

    std::sort(
        result.notes.begin(),
        result.notes.end(),
        [](const CompositionMidiNote& left,
           const CompositionMidiNote& right)
        {
            if (left.startTick !=
                right.startTick)
            {
                return left.startTick <
                       right.startTick;
            }

            if (left.channel !=
                right.channel)
            {
                return left.channel <
                       right.channel;
            }

            return left.midiNote <
                   right.midiNote;
        });

    result.lengthTicks =
        maximumEndTick;

    result.analysisValid =
        true;

    if (!result.isValid())
    {
        result.analysisValid =
            false;
    }

    return result;
}

} // namespace midigengx::music
