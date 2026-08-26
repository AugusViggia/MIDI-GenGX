#include "../Domain/CompositionIntent.h"

#include <cstdlib>
#include <iostream>

using namespace midigengx::domain;

namespace
{

void expect(
    bool condition,
    const char* message)
{
    if (!condition)
    {
        std::cerr
            << "FAIL: "
            << message
            << '\n';
        std::exit(1);
    }
}

void testDefaultIntentIsValid()
{
    CompositionIntent intent{};
    intent.normalize();

    expect(
        intent.isValid(),
        "default composition intent should be valid");

    expect(
        intent.duration.mode ==
            CompositionGenerationMode::Continuous,
        "default generation mode should be continuous");
}

void testCompleteGenerationDurationContract()
{
    CompositionIntent intent{};

    intent.duration.mode =
        CompositionGenerationMode::Complete;

    intent.duration.minimumSeconds = 30;
    intent.duration.maximumSeconds = 500;
    intent.duration.preferredSeconds = 700;

    intent.normalize();

    expect(
        intent.duration.minimumSeconds == 60,
        "AI complete mode minimum must normalize to 60 seconds");

    expect(
        intent.duration.maximumSeconds == 360,
        "AI complete mode maximum must normalize to 360 seconds");

    expect(
        intent.duration.preferredSeconds == 360,
        "preferred duration must remain inside the normalized range");

    expect(
        intent.duration.isValid(),
        "normalized complete duration must be valid");
}

void testComposerInfluenceNormalization()
{
    CompositionIntent intent{};

    intent.composerInfluences.push_back({
        "Chopin",
        150
    });

    intent.composerInfluences.push_back({
        "Bach",
        -5
    });

    intent.normalize();

    expect(
        intent.composerInfluences[0].weight == 100,
        "composer influence must clamp at 100");

    expect(
        intent.composerInfluences[1].weight == 0,
        "composer influence must clamp at 0");

    expect(
        intent.isValid(),
        "normalized composer influences must remain valid");
}

void testPriorityContract()
{
    CompositionIntent intent{};

    intent.selectorPriority = 20;
    intent.promptPriority = 30;
    intent.genreKnowledgePriority = 40;
    intent.composerKnowledgePriority = 50;
    intent.generationPreferencePriority = 60;

    expect(
        intent.isValid(),
        "default priority ordering should be valid");

    intent.composerKnowledgePriority = 25;

    expect(
        !intent.isValid(),
        "composer priority inversion must invalidate intent");
}

void testEngineeringAndStructureNormalization()
{
    CompositionIntent intent{};

    intent.structure.structuralContrast = 140;
    intent.structure.energyDevelopment = -10;

    intent.soundEngineering.lowEndOrganization = -20;
    intent.soundEngineering.densityBudget = 150;

    intent.normalize();

    expect(
        intent.structure.structuralContrast == 100,
        "structure contrast must clamp at 100");

    expect(
        intent.structure.energyDevelopment == 0,
        "energy development must clamp at 0");

    expect(
        intent.soundEngineering.lowEndOrganization == 0,
        "low-end organization must clamp at 0");

    expect(
        intent.soundEngineering.densityBudget == 100,
        "density budget must clamp at 100");
}

} // namespace

int main()
{
    testDefaultIntentIsValid();
    testCompleteGenerationDurationContract();
    testComposerInfluenceNormalization();
    testPriorityContract();
    testEngineeringAndStructureNormalization();

    std::cout
        << "CompositionIntentTests passed.\n";

    return 0;
}
