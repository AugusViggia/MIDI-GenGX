#include "Music/CompositionConditionedSequenceNeuralModelArtifactFileInspector.h"

#include <cstdlib>
#include <iostream>

using namespace midigengx::music;

namespace
{

void fail(const char* message)
{
    std::cerr << "FAILED: " << message << '\n';
    std::exit(1);
}

} // namespace

int main(int argc, char* argv[])
{
    if (argc != 2)
        fail("usage: Phase120RealArtifactInspection <model.mgcn>");

    const auto result =
        inspectCompositionConditionedSequenceNeuralModelArtifactFile(
            argv[1]);

    if (!result.isValid())
        fail("real Phase 119 model artifact inspection failed");

    std::cout
        << "PHASE 120 REAL ARTIFACT INSPECTION COMPLETE\n"
        << "valid=1\n"
        << "fileByteCount=" << result.fileByteCount << '\n'
        << "artifactVersion=" << result.artifactVersion << '\n'
        << "composer=" << result.composerSummary << '\n'
        << "style=" << result.styleSummary << '\n'
        << "era=" << result.eraSummary << '\n'
        << "instrumentation=" << result.instrumentationSummary << '\n';

    return 0;
}
