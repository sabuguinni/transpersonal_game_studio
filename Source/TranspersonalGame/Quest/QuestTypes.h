#pragma once

#include "CoreMinimal.h"
#include "QuestTypes.generated.h"

// ============================================================
// Quest & Mission System — Shared Types
// Agent #14 — Quest & Mission Designer
// Cycle: PROD_CYCLE_AUTO_20260628_005
// ============================================================

UENUM(BlueprintType)
enum class EQuest_Status : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Active          UMETA(DisplayName = "Active"),
    ObjectivesMet   UMETA(DisplayName = "Objectives Met"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Abandoned       UMETA(DisplayName = "Abandoned")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    ReachLocation       UMETA(DisplayName = "Reach Location"),
    KillTarget          UMETA(DisplayName = "Kill Target"),
    CollectItem         UMETA(DisplayName = "Collect Item"),
    EscapeArea          UMETA(DisplayName = "Escape Area"),
    ProtectNPC          UMETA(DisplayName = "Protect NPC"),
    SurviveDuration     UMETA(DisplayName = "Survive Duration"),
    CraftItem           UMETA(DisplayName = "Craft Item"),
    ScoutLocation       UMETA(DisplayName = "Scout Location"),
    TrackAnimal         UMETA(DisplayName = "Track Animal"),
    DeliverItem         UMETA(DisplayName = "Deliver Item")
};

UENUM(BlueprintType)
enum class EQuest_Difficulty : uint8
{
    Scavenger   UMETA(DisplayName = "Scavenger"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Apex        UMETA(DisplayName = "Apex")
};

UENUM(BlueprintType)
enum class EQuest_Category : uint8
{
    Survival    UMETA(DisplayName = "Survival"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Exploration UMETA(DisplayName = "Exploration"),
    Defense     UMETA(DisplayName = "Defense"),
    Rescue      UMETA(DisplayName = "Rescue"),
    Crafting    UMETA(DisplayName = "Crafting"),
    Migration   UMETA(DisplayName = "Migration")
};

USTRUCT(BlueprintType)
struct FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsOptional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float LocationRadius;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , ObjectiveType(EQuest_ObjectiveType::ReachLocation)
        , RequiredCount(1)
        , CurrentCount(0)
        , bIsOptional(false)
        , bIsCompleted(false)
        , TargetLocation(FVector::ZeroVector)
        , LocationRadius(500.f)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Reward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ExperiencePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString UnlockedRecipe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString UnlockedArea;

    FQuest_Reward()
        : ExperiencePoints(0)
        , UnlockedRecipe(TEXT(""))
        , UnlockedArea(TEXT(""))
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Data
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Category Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Difficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Status Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_Reward Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestGiverID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsMainQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> PrerequisiteQuestIDs;

    FQuest_Data()
        : QuestID(TEXT(""))
        , QuestTitle(TEXT(""))
        , QuestDescription(TEXT(""))
        , Category(EQuest_Category::Survival)
        , Difficulty(EQuest_Difficulty::Scavenger)
        , Status(EQuest_Status::Inactive)
        , QuestGiverID(TEXT(""))
        , bIsMainQuest(false)
        , TimeLimit(0.f)
    {}
};
