#pragma once

#include "CoreMinimal.h"
#include "QuestSystemTypes.generated.h"

// ============================================================
// Quest System Types — Agent #14 Quest & Mission Designer
// Cycle: PROD_CYCLE_AUTO_20260629_005
// All types prefixed with Quest_ to avoid collisions
// ============================================================

UENUM(BlueprintType)
enum class EQuest_Status : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed"),
    Abandoned   UMETA(DisplayName = "Abandoned")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Hunt        UMETA(DisplayName = "Hunt Target"),
    Collect     UMETA(DisplayName = "Collect Resource"),
    Reach       UMETA(DisplayName = "Reach Location"),
    Survive     UMETA(DisplayName = "Survive Duration"),
    Protect     UMETA(DisplayName = "Protect NPC"),
    Craft       UMETA(DisplayName = "Craft Item"),
    Scout       UMETA(DisplayName = "Scout Area"),
    Destroy     UMETA(DisplayName = "Destroy Target")
};

UENUM(BlueprintType)
enum class EQuest_Tier : uint8
{
    Survival    UMETA(DisplayName = "Survival"),   // Basic needs quests
    Hunt        UMETA(DisplayName = "Hunt"),        // Dinosaur hunting quests
    Explore     UMETA(DisplayName = "Explore"),     // Discovery quests
    Defend      UMETA(DisplayName = "Defend"),      // Camp defense quests
    Migrate     UMETA(DisplayName = "Migrate")      // Herd following quests
};

USTRUCT(BlueprintType)
struct FQuest_Reward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
    int32 BoneCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
    int32 HideCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
    int32 MeatCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
    int32 StoneCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
    float XPAmount = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
    TArray<FName> UnlockedRecipes;
};

USTRUCT(BlueprintType)
struct FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    EQuest_ObjectiveType ObjectiveType = EQuest_ObjectiveType::Hunt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    FName TargetTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    int32 RequiredCount = 1;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Objective")
    int32 CurrentCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    bool bIsOptional = false;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Objective")
    bool bIsCompleted = false;

    // Location-based objective data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    float LocationRadius = 500.0f;

    // Survival timer (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    float SurvivalDuration = 0.0f;
};

USTRUCT(BlueprintType)
struct FQuest_Data
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FName QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Tier Tier = EQuest_Tier::Survival;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Status Status = EQuest_Status::Inactive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_Reward Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FName GiverNPCTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsMainQuest = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FName> PrerequisiteQuestIDs;

    // Time limit in seconds (0 = no limit)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimitSeconds = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    float ElapsedTimeSeconds = 0.0f;
};
