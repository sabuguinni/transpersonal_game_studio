#pragma once

#include "CoreMinimal.h"
#include "QuestSystemTypes.generated.h"

// ============================================================
// Quest System Types — Agent #14 Quest & Mission Designer
// Cycle: PROD_CYCLE_AUTO_20260629_011
// All types prefixed with Quest_ to avoid collision
// ============================================================

UENUM(BlueprintType)
enum class EQuest_Status : uint8
{
    Locked       UMETA(DisplayName = "Locked"),
    Available    UMETA(DisplayName = "Available"),
    Active       UMETA(DisplayName = "Active"),
    Completed    UMETA(DisplayName = "Completed"),
    Failed       UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Hunt         UMETA(DisplayName = "Hunt Target"),
    Gather       UMETA(DisplayName = "Gather Resources"),
    Explore      UMETA(DisplayName = "Explore Location"),
    Defend       UMETA(DisplayName = "Defend Camp"),
    Escort       UMETA(DisplayName = "Escort NPC"),
    Craft        UMETA(DisplayName = "Craft Item"),
    Track        UMETA(DisplayName = "Track Animal"),
    Survive      UMETA(DisplayName = "Survive Duration")
};

UENUM(BlueprintType)
enum class EQuest_Difficulty : uint8
{
    Scavenger    UMETA(DisplayName = "Scavenger"),
    Hunter       UMETA(DisplayName = "Hunter"),
    Apex         UMETA(DisplayName = "Apex Predator")
};

UENUM(BlueprintType)
enum class EQuest_RewardType : uint8
{
    Resources    UMETA(DisplayName = "Resources"),
    Tools        UMETA(DisplayName = "Tools"),
    Knowledge    UMETA(DisplayName = "Knowledge"),
    Shelter      UMETA(DisplayName = "Shelter Access"),
    Alliance     UMETA(DisplayName = "Tribe Alliance")
};

// Single objective within a quest
USTRUCT(BlueprintType)
struct FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString TargetTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    bool bCompleted;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , ObjectiveType(EQuest_ObjectiveType::Hunt)
        , TargetTag(TEXT(""))
        , RequiredCount(1)
        , CurrentCount(0)
        , bCompleted(false)
    {}
};

// Reward granted on quest completion
USTRUCT(BlueprintType)
struct FQuest_Reward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_RewardType RewardType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText RewardDescription;

    FQuest_Reward()
        : RewardType(EQuest_RewardType::Resources)
        , ItemID(TEXT(""))
        , Quantity(1)
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
    FText QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText QuestNarrative;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Difficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Reward> Rewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> PrerequisiteQuestIDs;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    EQuest_Status Status;

    // Optional time limit in seconds (0 = no limit)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimitSeconds;

    // NPC giver tag
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString GiverNPCTag;

    FQuest_Definition()
        : QuestID(TEXT(""))
        , Difficulty(EQuest_Difficulty::Hunter)
        , Status(EQuest_Status::Locked)
        , TimeLimitSeconds(0.0f)
        , GiverNPCTag(TEXT(""))
    {}
};

// Runtime quest state tracked per player
USTRUCT(BlueprintType)
struct FQuest_RuntimeState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    FString QuestID;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    EQuest_Status Status;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TArray<FQuest_Objective> ActiveObjectives;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    float ElapsedTime;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    float StartTime;

    FQuest_RuntimeState()
        : QuestID(TEXT(""))
        , Status(EQuest_Status::Locked)
        , ElapsedTime(0.0f)
        , StartTime(0.0f)
    {}
};
