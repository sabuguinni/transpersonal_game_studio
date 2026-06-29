#pragma once

#include "CoreMinimal.h"
#include "QuestSystemTypes.generated.h"

// ============================================================
// Quest System Shared Types — Agent #14 Quest & Mission Designer
// Cycle: PROD_CYCLE_AUTO_20260629_010
// All types prefixed with EQuest_ / FQuest_ / UQuest_ per UBT rules
// ============================================================

// Quest state machine
UENUM(BlueprintType)
enum class EQuest_State : uint8
{
    Locked          UMETA(DisplayName = "Locked"),
    Available       UMETA(DisplayName = "Available"),
    Active          UMETA(DisplayName = "Active"),
    Paused          UMETA(DisplayName = "Paused"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Abandoned       UMETA(DisplayName = "Abandoned")
};

// Quest category — drives UI grouping and journal sorting
UENUM(BlueprintType)
enum class EQuest_Category : uint8
{
    MainStory       UMETA(DisplayName = "Main Story"),
    Survival        UMETA(DisplayName = "Survival"),
    Hunt            UMETA(DisplayName = "Hunt"),
    Exploration     UMETA(DisplayName = "Exploration"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Defense         UMETA(DisplayName = "Defense"),
    Rescue          UMETA(DisplayName = "Rescue"),
    Migration       UMETA(DisplayName = "Migration")
};

// Objective type — determines how completion is tracked
UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    ReachLocation       UMETA(DisplayName = "Reach Location"),
    KillTarget          UMETA(DisplayName = "Kill Target"),
    CollectItem         UMETA(DisplayName = "Collect Item"),
    CraftItem           UMETA(DisplayName = "Craft Item"),
    ProtectTarget       UMETA(DisplayName = "Protect Target"),
    EscapeArea          UMETA(DisplayName = "Escape Area"),
    SurviveDuration     UMETA(DisplayName = "Survive Duration"),
    TrackAnimal         UMETA(DisplayName = "Track Animal"),
    BuildStructure      UMETA(DisplayName = "Build Structure"),
    InteractWithNPC     UMETA(DisplayName = "Interact With NPC")
};

// Reward type
UENUM(BlueprintType)
enum class EQuest_RewardType : uint8
{
    ResourceBundle      UMETA(DisplayName = "Resource Bundle"),
    CraftingRecipe      UMETA(DisplayName = "Crafting Recipe"),
    WeaponUpgrade       UMETA(DisplayName = "Weapon Upgrade"),
    TribeRelation       UMETA(DisplayName = "Tribe Relation"),
    MapReveal           UMETA(DisplayName = "Map Reveal"),
    ShelterLocation     UMETA(DisplayName = "Shelter Location")
};

// Threat level from crowd system — used for quest trigger conditions
UENUM(BlueprintType)
enum class EQuest_ThreatCondition : uint8
{
    None            UMETA(DisplayName = "None"),
    NearbyPredator  UMETA(DisplayName = "Nearby Predator"),
    StampedeActive  UMETA(DisplayName = "Stampede Active"),
    NightFall       UMETA(DisplayName = "Night Fall"),
    PlayerLowHealth UMETA(DisplayName = "Player Low Health"),
    TribeHostile    UMETA(DisplayName = "Tribe Hostile")
};

// ============================================================
// STRUCTS
// ============================================================

// Single quest objective
USTRUCT(BlueprintType)
struct FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    FName ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    FText Description;

    // For location-based objectives
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    float LocationRadius;

    // For kill/collect objectives
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    FName TargetTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    int32 CurrentCount;

    // For survive duration objectives (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    float RequiredDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    float ElapsedDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    bool bIsOptional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    bool bIsCompleted;

    FQuest_Objective()
        : ObjectiveType(EQuest_ObjectiveType::ReachLocation)
        , TargetLocation(FVector::ZeroVector)
        , LocationRadius(300.f)
        , RequiredCount(1)
        , CurrentCount(0)
        , RequiredDuration(0.f)
        , ElapsedDuration(0.f)
        , bIsOptional(false)
        , bIsCompleted(false)
    {}
};

// Quest reward definition
USTRUCT(BlueprintType)
struct FQuest_Reward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
    EQuest_RewardType RewardType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
    FName RewardID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
    FText RewardDescription;

    FQuest_Reward()
        : RewardType(EQuest_RewardType::ResourceBundle)
        , Quantity(1)
    {}
};

// Full quest definition — the core data asset
USTRUCT(BlueprintType)
struct FQuest_Definition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FName QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Category Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_State State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Reward> Rewards;

    // Prerequisites — quest IDs that must be completed first
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FName> PrerequisiteQuestIDs;

    // NPC who gives this quest
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FName QuestGiverID;

    // Threat condition that can auto-trigger this quest
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ThreatCondition AutoTriggerCondition;

    // Time limit in seconds (0 = no limit)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimitSeconds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bFailOnPlayerDeath;

    FQuest_Definition()
        : Category(EQuest_Category::Survival)
        , State(EQuest_State::Locked)
        , AutoTriggerCondition(EQuest_ThreatCondition::None)
        , TimeLimitSeconds(0.f)
        , bIsRepeatable(false)
        , bFailOnPlayerDeath(true)
    {}
};

// Runtime quest tracker — tracks active quest progress
USTRUCT(BlueprintType)
struct FQuest_ActiveRecord
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Runtime")
    FName QuestID;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Runtime")
    EQuest_State CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Runtime")
    float TimeStarted;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Runtime")
    float TimeCompleted;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Runtime")
    TArray<FQuest_Objective> ActiveObjectives;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Runtime")
    int32 CompletedObjectiveCount;

    FQuest_ActiveRecord()
        : CurrentState(EQuest_State::Locked)
        , TimeStarted(0.f)
        , TimeCompleted(0.f)
        , CompletedObjectiveCount(0)
    {}
};
