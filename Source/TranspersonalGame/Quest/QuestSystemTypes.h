#pragma once

#include "CoreMinimal.h"
#include "QuestSystemTypes.generated.h"

// ============================================================
// Quest & Mission Designer — Agent #14
// QuestSystemTypes.h — All shared quest enums and structs
// Prefix: Quest_ (avoids collision with other agents)
// ============================================================

// Quest state machine
UENUM(BlueprintType)
enum class EQuest_State : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Active          UMETA(DisplayName = "Active"),
    ObjectiveReached UMETA(DisplayName = "Objective Reached"),
    Failed          UMETA(DisplayName = "Failed"),
    Completed       UMETA(DisplayName = "Completed")
};

// Quest category — determines UI grouping and reward type
UENUM(BlueprintType)
enum class EQuest_Category : uint8
{
    Survival        UMETA(DisplayName = "Survival"),      // Immediate danger — must act now
    Hunt            UMETA(DisplayName = "Hunt"),          // Track and kill a specific animal
    Rescue          UMETA(DisplayName = "Rescue"),        // Save an NPC before time runs out
    Exploration     UMETA(DisplayName = "Exploration"),   // Discover a new map area
    Crafting        UMETA(DisplayName = "Crafting"),      // Gather materials and craft item
    Defense         UMETA(DisplayName = "Defense"),       // Protect camp from predator attack
    Migration       UMETA(DisplayName = "Migration")      // Follow a herd to find resources
};

// Objective type — what the player must DO
UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    ReachLocation   UMETA(DisplayName = "Reach Location"),
    KillTarget      UMETA(DisplayName = "Kill Target"),
    CollectItem     UMETA(DisplayName = "Collect Item"),
    ProtectActor    UMETA(DisplayName = "Protect Actor"),
    SurviveDuration UMETA(DisplayName = "Survive Duration"),
    TrackTarget     UMETA(DisplayName = "Track Target"),
    CraftItem       UMETA(DisplayName = "Craft Item")
};

// Reward type
UENUM(BlueprintType)
enum class EQuest_RewardType : uint8
{
    Resources       UMETA(DisplayName = "Resources"),     // Stone, wood, bone, hide
    Knowledge       UMETA(DisplayName = "Knowledge"),     // Unlock new crafting recipe
    Shelter         UMETA(DisplayName = "Shelter"),       // New camp location unlocked
    Ally            UMETA(DisplayName = "Ally"),          // NPC joins the tribe
    TerritoryMap    UMETA(DisplayName = "Territory Map")  // New map area revealed
};

// Failure condition
UENUM(BlueprintType)
enum class EQuest_FailCondition : uint8
{
    PlayerDied      UMETA(DisplayName = "Player Died"),
    NPCDied         UMETA(DisplayName = "NPC Died"),
    TimerExpired    UMETA(DisplayName = "Timer Expired"),
    AreaLeft        UMETA(DisplayName = "Area Left"),
    StampedePassed  UMETA(DisplayName = "Stampede Passed") // Herd crossed before player reached safe zone
};

// ============================================================
// Single quest objective
// ============================================================
USTRUCT(BlueprintType)
struct FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType ObjectiveType = EQuest_ObjectiveType::ReachLocation;

    // Target actor label in the level (e.g. "QuestObjective_StampedeSafeZone")
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString TargetActorLabel;

    // For SurviveDuration objectives — seconds to survive
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float DurationSeconds = 0.0f;

    // For CollectItem / CraftItem — how many needed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount = 1;

    // Current progress
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    int32 CurrentCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    bool bIsCompleted = false;
};

// ============================================================
// Full quest definition
// ============================================================
USTRUCT(BlueprintType)
struct FQuest_Definition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Category Category = EQuest_Category::Survival;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_State State = EQuest_State::Inactive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_RewardType RewardType = EQuest_RewardType::Resources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString RewardDescription;

    // Time limit in seconds — 0 means no time limit
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimitSeconds = 0.0f;

    // Elapsed time since quest activation
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    float ElapsedSeconds = 0.0f;

    // Fail condition
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_FailCondition FailCondition = EQuest_FailCondition::PlayerDied;

    // NPC quest giver actor label (for dialogue trigger)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestGiverActorLabel;

    // Whether this quest is triggered by a crowd event (stampede, herd migration)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCrowdTriggered = false;

    // Crowd event ID that activates this quest (matches FCrowd_StampedeEvent.GroupID)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CrowdTriggerGroupID = -1;
};

// ============================================================
// Quest log entry (for HUD display)
// ============================================================
USTRUCT(BlueprintType)
struct FQuest_LogEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    FString QuestID;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    FString QuestTitle;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    EQuest_State State = EQuest_State::Inactive;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    FString ActiveObjectiveDescription;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    float TimeRemainingSeconds = 0.0f;
};
