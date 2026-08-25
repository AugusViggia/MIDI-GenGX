#include "MotifOccurrenceGraph.h"

namespace midigengx::music
{

bool MotifRelationshipEdge::isValid() const noexcept
{
    return sourceOccurrence != targetOccurrence &&
           relationship.isValid();
}

bool MotifOccurrenceGraph::isValid() const noexcept
{
    if (!analysisValid)
        return false;

    for (const auto& occurrence :
         occurrences)
    {
        if (!occurrence.fingerprint.isValid())
            return false;

        if (occurrence.occurrenceIndex >=
            occurrences.size())
        {
            return false;
        }
    }

    for (const auto& edge :
         edges)
    {
        if (!edge.isValid() ||
            edge.sourceOccurrence >=
                occurrences.size() ||
            edge.targetOccurrence >=
                occurrences.size())
        {
            return false;
        }
    }

    return true;
}

std::size_t MotifOccurrenceGraph::countOccurrencesOf(
    const MotifFingerprint& fingerprint) const noexcept
{
    if (!fingerprint.isValid())
        return 0;

    std::size_t count = 0;

    for (const auto& occurrence :
         occurrences)
    {
        if (isMotivicIdentityEquivalent(
                occurrence.fingerprint,
                fingerprint))
        {
            ++count;
        }
    }

    return count;
}

MotifOccurrenceGraph buildMotifOccurrenceGraph(
    const std::vector<Motif>& motifs,
    const std::vector<std::size_t>& sourcePhraseIndices) noexcept
{
    MotifOccurrenceGraph graph;
    graph.analysisValid = true;

    graph.occurrences.reserve(
        motifs.size());

    std::vector<std::size_t> originalIndices;
    originalIndices.reserve(
        motifs.size());

    for (std::size_t i = 0;
         i < motifs.size();
         ++i)
    {
        const auto fingerprint =
            fingerprintMotif(
                motifs[i]);

        if (!fingerprint.isValid())
            continue;

        MotifOccurrence occurrence;
        occurrence.occurrenceIndex =
            graph.occurrences.size();

        occurrence.sourcePhraseIndex =
            i < sourcePhraseIndices.size()
                ? sourcePhraseIndices[i]
                : i;

        occurrence.startBeat =
            motifs[i].notes.empty()
                ? 0.0
                : motifs[i].notes.front().startBeat;

        occurrence.fingerprint =
            fingerprint;

        graph.occurrences.push_back(
            occurrence);

        originalIndices.push_back(i);
    }

    for (std::size_t source = 0;
         source < originalIndices.size();
         ++source)
    {
        for (std::size_t target = source + 1;
             target < originalIndices.size();
             ++target)
        {
            const auto relationship =
                analyzeMotifRelationship(
                    motifs[
                        originalIndices[source]],
                    motifs[
                        originalIndices[target]]);

            if (!relationship.isValid())
                continue;

            MotifRelationshipEdge edge;
            edge.sourceOccurrence = source;
            edge.targetOccurrence = target;
            edge.relationship = relationship;

            graph.edges.push_back(edge);
        }
    }

    return graph;
}

} // namespace midigengx::music
