#include "CompositionGenreKnowledgeRepresentation.h"

#include <algorithm>
#include <cmath>

namespace midigengx::music
{
namespace
{

using midigengx::domain::CadenceStyle;
using midigengx::domain::GenrePreset;
using midigengx::domain::NoteLength;
using midigengx::domain::PhraseContour;

bool isUnit(double value) noexcept
{
    return std::isfinite(value) &&
           value >= 0.0 &&
           value <= 1.0;
}

double percent(int value) noexcept
{
    return std::clamp(value, 0, 100) / 100.0;
}

double noteLengthEncoding(NoteLength value) noexcept
{
    switch (value)
    {
        case NoteLength::Short:    return 0.00;
        case NoteLength::Medium:   return 0.20;
        case NoteLength::Long:     return 0.40;
        case NoteLength::Legato:   return 0.60;
        case NoteLength::Staccato: return 0.80;
        case NoteLength::Custom:   return 1.00;
        case NoteLength::Auto:
        default:                   return 0.10;
    }
}

double phraseContourEncoding(PhraseContour value) noexcept
{
    switch (value)
    {
        case PhraseContour::Arch:       return 0.00;
        case PhraseContour::Ascending:  return 0.20;
        case PhraseContour::Descending: return 0.40;
        case PhraseContour::Flat:       return 0.60;
        case PhraseContour::Valley:     return 0.80;
        case PhraseContour::Custom:     return 1.00;
        default:                        return 0.00;
    }
}

double cadenceStyleEncoding(CadenceStyle value) noexcept
{
    switch (value)
    {
        case CadenceStyle::Root:   return 0.00;
        case CadenceStyle::Fifth:  return 0.25;
        case CadenceStyle::Third:  return 0.50;
        case CadenceStyle::Open:   return 0.75;
        case CadenceStyle::Custom: return 1.00;
        default:                   return 0.00;
    }
}

double average(double a, double b) noexcept
{
    return std::clamp((a + b) * 0.5, 0.0, 1.0);
}

} // namespace

bool CompositionGenreKnowledgeRepresentation::isValid() const noexcept
{
    if (!valid ||
        genre == GenrePreset::Count)
    {
        return false;
    }

    return std::all_of(
        features.begin(),
        features.end(),
        [](double value)
        {
            return isUnit(value);
        });
}

CompositionGenreKnowledgeRepresentation
buildCompositionGenreKnowledgeRepresentation(
    GenrePreset genre) noexcept
{
    CompositionGenreKnowledgeRepresentation result;

    if (genre == GenrePreset::Count)
        return result;

    const auto& preset =
        getGenrePresetValues(genre);

    result.genre = genre;

    result.features[CompositionGenreKnowledgeRepresentation::Density] =
        percent(preset.density);
    result.features[CompositionGenreKnowledgeRepresentation::Variation] =
        percent(preset.variation);
    result.features[CompositionGenreKnowledgeRepresentation::Complexity] =
        percent(preset.complexity);
    result.features[CompositionGenreKnowledgeRepresentation::Syncopation] =
        percent(preset.syncopation);
    result.features[CompositionGenreKnowledgeRepresentation::Tension] =
        percent(preset.tension);
    result.features[CompositionGenreKnowledgeRepresentation::Repetition] =
        percent(preset.repetition);
    result.features[CompositionGenreKnowledgeRepresentation::Humanization] =
        percent(preset.humanization);
    result.features[CompositionGenreKnowledgeRepresentation::NoteLengthVariation] =
        percent(preset.noteLengthVariation);
    result.features[CompositionGenreKnowledgeRepresentation::CadenceStrength] =
        percent(preset.cadenceStrength);
    result.features[CompositionGenreKnowledgeRepresentation::NoteLengthEncoding] =
        noteLengthEncoding(preset.noteLength);
    result.features[CompositionGenreKnowledgeRepresentation::PhraseContourEncoding] =
        phraseContourEncoding(preset.phraseContour);
    result.features[CompositionGenreKnowledgeRepresentation::CadenceStyleEncoding] =
        cadenceStyleEncoding(preset.cadenceStyle);

    // These derived dimensions deliberately remain deterministic and are
    // composed only from already-established genre preset values. They are
    // not claims of universal genre standards and are versioned as part of
    // the representation contract.
    result.features[CompositionGenreKnowledgeRepresentation::EnergyProfile] =
        average(
            result.features[CompositionGenreKnowledgeRepresentation::Density],
            result.features[CompositionGenreKnowledgeRepresentation::Tension]);

    result.features[CompositionGenreKnowledgeRepresentation::StructuralProfile] =
        average(
            result.features[CompositionGenreKnowledgeRepresentation::Repetition],
            result.features[CompositionGenreKnowledgeRepresentation::Variation]);

    result.features[CompositionGenreKnowledgeRepresentation::HarmonicProfile] =
        average(
            result.features[CompositionGenreKnowledgeRepresentation::Tension],
            result.features[CompositionGenreKnowledgeRepresentation::CadenceStrength]);

    result.valid = true;

    if (!result.isValid())
        result.valid = false;

    return result;
}

} // namespace midigengx::music
