#pragma once

#include "GenerationIntent.h"

#include <algorithm>
#include <string>
#include <vector>

namespace midigengx::domain
{

enum class CompositionGenerationMode
{
    Continuous,
    Complete
};

enum class ComposerInfluenceStrength
{
    Subtle = 25,
    Moderate = 50,
    Strong = 75,
    Dominant = 100
};

struct ComposerInfluence
{
    std::string composerId;
    int weight = 50;

    void normalize() noexcept
    {
        weight = std::clamp(weight, 0, 100);
    }

    bool isValid() const noexcept
    {
        return !composerId.empty()
            && weight >= 0
            && weight <= 100;
    }
};

struct CompositionDurationIntent
{
    CompositionGenerationMode mode =
        CompositionGenerationMode::Continuous;

    int minimumSeconds = 0;
    int maximumSeconds = 0;
    int preferredSeconds = 0;

    void normalize() noexcept
    {
        minimumSeconds =
            std::max(0, minimumSeconds);

        maximumSeconds =
            std::max(
                minimumSeconds,
                maximumSeconds);

        preferredSeconds =
            std::clamp(
                preferredSeconds,
                minimumSeconds,
                maximumSeconds);

        if (mode ==
            CompositionGenerationMode::Complete)
        {
            minimumSeconds =
                std::clamp(
                    minimumSeconds,
                    60,
                    360);

            maximumSeconds =
                std::clamp(
                    maximumSeconds,
                    minimumSeconds,
                    360);

            preferredSeconds =
                std::clamp(
                    preferredSeconds,
                    minimumSeconds,
                    maximumSeconds);
        }
    }

    bool isValid() const noexcept
    {
        if (minimumSeconds < 0 ||
            maximumSeconds < minimumSeconds ||
            preferredSeconds < minimumSeconds ||
            preferredSeconds > maximumSeconds)
        {
            return false;
        }

        if (mode ==
            CompositionGenerationMode::Complete)
        {
            return minimumSeconds >= 60
                && maximumSeconds <= 360;
        }

        return true;
    }
};

struct CompositionStructureIntent
{
    // Ordered section names are intentionally free-form.
    // Genre knowledge will resolve them in a later phase.
    std::vector<std::string> preferredSections;

    int structuralContrast = 50;
    int energyDevelopment = 50;
    int sectionVariation = 50;

    bool allowAutoStructure = true;

    void normalize() noexcept
    {
        structuralContrast =
            std::clamp(
                structuralContrast,
                0,
                100);

        energyDevelopment =
            std::clamp(
                energyDevelopment,
                0,
                100);

        sectionVariation =
            std::clamp(
                sectionVariation,
                0,
                100);
    }
};

struct SoundEngineeringIntent
{
    int lowEndOrganization = 75;
    int registerSeparation = 75;
    int rhythmicSpace = 50;
    int densityBudget = 50;
    int energyControl = 50;
    int grooveFocus = 50;

    bool prioritizeClarity = true;
    bool protectLowEnd = true;

    void normalize() noexcept
    {
        lowEndOrganization =
            clampPercent(lowEndOrganization);
        registerSeparation =
            clampPercent(registerSeparation);
        rhythmicSpace =
            clampPercent(rhythmicSpace);
        densityBudget =
            clampPercent(densityBudget);
        energyControl =
            clampPercent(energyControl);
        grooveFocus =
            clampPercent(grooveFocus);
    }

private:
    static int clampPercent(int value) noexcept
    {
        return std::clamp(value, 0, 100);
    }
};

struct CompositionIntent
{
    static constexpr int version = 1;

    // Existing domain intent remains the authoritative source for
    // explicit musical context such as key/scale and the current UI controls.
    GenerationIntent base{};

    // Natural-language creative intent.
    std::string prompt;

    // Composer knowledge is additive and weighted; it does not override
    // explicit hard musical constraints.
    std::vector<ComposerInfluence> composerInfluences;

    // Genre tags are intentionally string-based at this boundary so the
    // future genre knowledge system can support aliases/subgenres without
    // changing this schema.
    std::vector<std::string> genreTags;

    CompositionStructureIntent structure{};
    SoundEngineeringIntent soundEngineering{};
    CompositionDurationIntent duration{};

    // Hard constraints must always survive interpretation.
    bool keyIsHardConstraint = true;
    bool scaleIsHardConstraint = true;
    bool roleIsHardConstraint = true;
    bool registerIsHardConstraint = true;

    // Relative precedence for soft sources. Lower number = higher priority.
    int selectorPriority = 20;
    int promptPriority = 30;
    int genreKnowledgePriority = 40;
    int composerKnowledgePriority = 50;
    int generationPreferencePriority = 60;

    void normalize() noexcept
    {
        base.normalize();

        for (auto& composer :
             composerInfluences)
        {
            composer.normalize();
        }

        structure.normalize();
        soundEngineering.normalize();
        duration.normalize();

        selectorPriority =
            std::clamp(selectorPriority, 0, 100);
        promptPriority =
            std::clamp(promptPriority, 0, 100);
        genreKnowledgePriority =
            std::clamp(genreKnowledgePriority, 0, 100);
        composerKnowledgePriority =
            std::clamp(composerKnowledgePriority, 0, 100);
        generationPreferencePriority =
            std::clamp(generationPreferencePriority, 0, 100);
    }

    bool isValid() const noexcept
    {
        if (version != 1 ||
            !duration.isValid())
        {
            return false;
        }

        for (const auto& composer :
             composerInfluences)
        {
            if (!composer.isValid())
                return false;
        }

        return selectorPriority <
                   promptPriority
            && promptPriority <
                   genreKnowledgePriority
            && genreKnowledgePriority <
                   composerKnowledgePriority
            && composerKnowledgePriority <
                   generationPreferencePriority;
    }
};

} // namespace midigengx::domain
