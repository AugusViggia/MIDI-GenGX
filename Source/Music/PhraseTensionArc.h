#pragma once

#include "../Domain/MusicalContext.h"

namespace midigengx::music
{

struct PhraseTensionArc
{
    static double normalizedProgress(
        double beat,
        double totalLengthBeats) noexcept;

    static double climaxPosition(
        midigengx::domain::PhraseContour contour) noexcept;

    static int tensionAtProgress(
        double progress,
        midigengx::domain::PhraseContour contour,
        int baseTension,
        int cadenceStrength) noexcept;

    static int sectionTension(
        double startBeat,
        double endBeat,
        double totalLengthBeats,
        midigengx::domain::PhraseContour contour,
        int baseTension,
        int cadenceStrength,
        bool cadenceSection) noexcept;
};

} // namespace midigengx::music
