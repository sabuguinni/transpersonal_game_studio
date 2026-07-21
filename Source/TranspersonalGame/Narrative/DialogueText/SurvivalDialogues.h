#pragma once

#include "CoreMinimal.h"

/**
 * Static dialogue text library for prehistoric survival scenarios
 * Contains all narrative content for NPCs, environmental storytelling, and quest guidance
 */
class TRANSPERSONALGAME_API FSurvivalDialogues
{
public:
    // Hunter Character Dialogues
    static const FString HUNTER_GREETING;
    static const FString HUNTER_WARNING_PREDATOR;
    static const FString HUNTER_ADVICE_TRACKS;
    static const FString HUNTER_SHELTER_TIP;
    static const FString HUNTER_WATER_LOCATION;

    // Tribal Elder Dialogues  
    static const FString ELDER_WISDOM_SURVIVAL;
    static const FString ELDER_WEATHER_READING;
    static const FString ELDER_ANCIENT_KNOWLEDGE;
    static const FString ELDER_DANGER_SIGNS;
    static const FString ELDER_TRIBAL_HISTORY;

    // Environmental Narration
    static const FString NARR_RIVER_DANGER;
    static const FString NARR_CAVE_DISCOVERY;
    static const FString NARR_PREDATOR_APPROACH;
    static const FString NARR_SAFE_SHELTER;
    static const FString NARR_RESOURCE_FOUND;

    // Quest Guidance
    static const FString QUEST_GATHER_MATERIALS;
    static const FString QUEST_FIND_WATER;
    static const FString QUEST_AVOID_PREDATORS;
    static const FString QUEST_BUILD_SHELTER;
    static const FString QUEST_CRAFT_TOOLS;

    // Survival Tips
    static const FString TIP_FIRE_MAKING;
    static const FString TIP_FOOD_PRESERVATION;
    static const FString TIP_WEATHER_PROTECTION;
    static const FString TIP_PREDATOR_AVOIDANCE;
    static const FString TIP_RESOURCE_CONSERVATION;

    // Emergency Warnings
    static const FString WARNING_STORM_APPROACHING;
    static const FString WARNING_PREDATOR_NEARBY;
    static const FString WARNING_UNSTABLE_GROUND;
    static const FString WARNING_TOXIC_PLANTS;
    static const FString WARNING_TERRITORY_BOUNDARY;
};