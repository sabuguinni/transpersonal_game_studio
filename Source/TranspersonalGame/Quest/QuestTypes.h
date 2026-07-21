#pragma once

#include "CoreMinimal.h"
#include "QuestTypes.generated.h"

// ============================================================
// QuestTypes.h — Agent #14 Quest & Mission Designer
// Cycle: PROD_CYCLE_AUTO_20260629_012
// All quest enums and structs for the TranspersonalGame quest system.
// Prefix: Quest_ (unique across project per RULE 2)
// ============================================================

// Quest state machine
UENUM(BlueprintType)
enum class EQuest_State : uint8
{
    Unavailable     UMETA(DisplayName = "Unavailable"),
    Available       UMETA(DisplayName = "Available"),
    Active          UMETA(DisplayName = "Active"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Abandoned       UMETA(DisplayName = "Abandoned")
};

// Quest category — survival-focused
UENUM(BlueprintType)
enum class EQuest_Category : uint8
{
    Hunt            UMETA(DisplayName = "Hunt"),
    Gather          UMETA(DisplayName = "Gather"),
    Rescue          UMETA(DisplayName = "Rescue"),
    Defend          UMETA(DisplayName = "Defend"),
    Explore         UMETA(DisplayName = "Explore"),
    Craft           UMETA(DisplayName = "Craft"),
    Survive         UMETA(DisplayName = "Survive"),
    Track           UMETA(DisplayName = "Track"),
    Migrate         UMETA(DisplayName = "Migrate")
};

// Objective type — what the player must do
UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    ReachLocation       UMETA(DisplayName = "Reach Location"),
    KillTarget          UMETA(DisplayName = "Kill Target"),
    CollectItem         UMETA(DisplayName = "Collect Item"),
    CraftItem           UMETA(DisplayName = "Craft Item"),
    ProtectNPC          UMETA(DisplayName = "Protect NPC"),
    EscapeArea          UMETA(DisplayName = "Escape Area"),
    TrackCreature       UMETA(DisplayName = "Track Creature"),
    SurviveDuration     UMETA(DisplayName = "Survive Duration"),
    ScoutLocation       UMETA(DisplayName = "Scout Location"),
    BuildStructure      UMETA(DisplayName = "Build Structure")
};

// Threat level that triggers or escalates a quest
UENUM(BlueprintType)
enum class EQuest_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical — Stampede / Alpha Predator")
};

// Single quest objective
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

    // For kill/collect objectives
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount = 0;

    // For location objectives
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float LocationRadius = 300.0f;

    // For timed objectives
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float RequiredDuration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float ElapsedDuration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsOptional = false;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , ObjectiveType(EQuest_ObjectiveType::ReachLocation)
        , RequiredCount(1)
        , CurrentCount(0)
        , TargetLocation(FVector::ZeroVector)
        , LocationRadius(300.0f)
        , RequiredDuration(0.0f)
        , ElapsedDuration(0.0f)
        , bIsCompleted(false)
        , bIsOptional(false)
    {}
};

// Quest reward — survival resources
USTRUCT(BlueprintType)
struct FQuest_Reward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 FoodAmount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 WaterAmount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 FlintCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 HideCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 BoneCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString UnlockedRecipeID;

    FQuest_Reward()
        : FoodAmount(0)
        , WaterAmount(0)
        , FlintCount(0)
        , HideCount(0)
        , BoneCount(0)
        , UnlockedRecipeID(TEXT(""))
    {}
};

// Full quest definition
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
    EQuest_Category Category = EQuest_Category::Survive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_State State = EQuest_State::Unavailable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ThreatLevel ThreatLevel = EQuest_ThreatLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_Reward Reward;

    // Linked to crowd/stampede system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bTriggeredByStampede = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector QuestOriginLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit = 0.0f; // 0 = no time limit

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> PrerequisiteQuestIDs;

    FQuest_Definition()
        : QuestID(TEXT(""))
        , QuestTitle(TEXT(""))
        , QuestDescription(TEXT(""))
        , Category(EQuest_Category::Survive)
        , State(EQuest_State::Unavailable)
        , ThreatLevel(EQuest_ThreatLevel::None)
        , bTriggeredByStampede(false)
        , QuestOriginLocation(FVector::ZeroVector)
        , TimeLimit(0.0f)
    {}
};

// Active quest tracker — runtime state
USTRUCT(BlueprintType)
struct FQuest_ActiveTracker
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    FString QuestID;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    float StartTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    float LastUpdateTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    int32 CompletedObjectiveCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    bool bAllObjectivesComplete = false;

    FQuest_ActiveTracker()
        : QuestID(TEXT(""))
        , StartTime(0.0f)
        , LastUpdateTime(0.0f)
        , CompletedObjectiveCount(0)
        , bAllObjectivesComplete(false)
    {}
};
