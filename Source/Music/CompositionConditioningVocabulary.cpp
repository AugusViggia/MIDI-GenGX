#include "CompositionConditioningVocabulary.h"

#include <algorithm>

namespace midigengx::music
{
namespace
{

template <typename Container>
std::uint32_t findValue(
    const Container& values,
    const std::string& value) noexcept
{
    const auto iterator =
        std::lower_bound(
            values.begin(),
            values.end(),
            value);

    if (iterator == values.end() ||
        *iterator != value)
    {
        return 0;
    }

    return static_cast<std::uint32_t>(
        std::distance(
            values.begin(),
            iterator));
}

template <typename Container>
void addUnique(
    Container& values,
    const std::string& value)
{
    if (value.empty())
        return;

    values.push_back(
        value);
}

template <typename Container>
void normalize(
    Container& values)
{
    std::sort(
        values.begin(),
        values.end());

    values.erase(
        std::unique(
            values.begin(),
            values.end()),
        values.end());
}

} // namespace

bool CompositionConditioningVocabulary::isValid()
    const noexcept
{
    return valid &&
           !composers.empty() &&
           !styles.empty() &&
           !eras.empty() &&
           !instrumentations.empty();
}

std::uint32_t CompositionConditioningVocabulary::composerIndex(
    const std::string& value) const noexcept
{
    return findValue(
        composers,
        value);
}

std::uint32_t CompositionConditioningVocabulary::styleIndex(
    const std::string& value) const noexcept
{
    return findValue(
        styles,
        value);
}

std::uint32_t CompositionConditioningVocabulary::eraIndex(
    const std::string& value) const noexcept
{
    return findValue(
        eras,
        value);
}

std::uint32_t CompositionConditioningVocabulary::instrumentationIndex(
    const std::string& value) const noexcept
{
    return findValue(
        instrumentations,
        value);
}

CompositionConditioningVocabulary
buildCompositionConditioningVocabulary(
    const std::vector<CompositionSequenceMetadata>& metadata)
    noexcept
{
    CompositionConditioningVocabulary vocabulary;

    if (metadata.empty())
        return vocabulary;

    for (const auto& entry :
         metadata)
    {
        if (!entry.isValid() ||
            !entry.verified)
        {
            return vocabulary;
        }

        addUnique(
            vocabulary.composers,
            entry.composerId);

        addUnique(
            vocabulary.styles,
            entry.styleId);

        addUnique(
            vocabulary.eras,
            entry.eraId);

        addUnique(
            vocabulary.instrumentations,
            entry.instrumentationId);
    }

    normalize(
        vocabulary.composers);

    normalize(
        vocabulary.styles);

    normalize(
        vocabulary.eras);

    normalize(
        vocabulary.instrumentations);

    vocabulary.valid =
        true;

    if (!vocabulary.isValid())
        vocabulary.valid = false;

    return vocabulary;
}

} // namespace midigengx::music
