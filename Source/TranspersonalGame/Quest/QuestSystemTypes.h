#pragma once

#include "CoreMinimal.h"
#include "QuestSystemTypes.generated.h"

// ============================================================
// Quest System Types — Agent #14 Quest & Mission Designer
// Cycle: PROD_CYCLE_AUTO_20260629_013
// All types prefixed with Quest_ to avoid collisions
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
    Hunt            UMETA(DisplayName = "Hunt Target"),
    Gather          UMETA(DisplayName = "Gather Resources"),
    Explore         UMETA(DisplayName = "Explore Location"),
    Defend          UMETA(DisplayName = "Defend Camp"),
    Rescue          UMETA(DisplayName = "Rescue NPC"),
    Craft           UMETA(DisplayName = "Craft Item"),
    Track           UMETA(DisplayName = "Track Animal"),
    Survive         UMETA(DisplayName = "Survive Duration"),
    Escort          UMETA(DisplayName = "Escort NPC"),
    Investigate     UMETA(DisplayName = "Investigate Site")
};

UENUM(BlueprintType)
enum class EQuest_Priority : uint8
{
    Critical        UMETA(DisplayName = "Critical"),
    High            UMETA(DisplayName = "High"),
    Normal          UMETA(DisplayName = "Normal"),
    Low             UMETA(DisplayName = "Low"),
    Optional        UMETA(DisplayName = "Optional")
};

UENUM(BlueprintType)
enum class EQuest_RewardType : uint8
{
    Resources       UMETA(DisplayName = "Resources"),
    Tools           UMETA(DisplayName = "Tools"),
    Knowledge       UMETA(DisplayName = "Knowledge"),
    Shelter         UMETA(DisplayName = "Shelter Access"),
    Allies          UMETA(DisplayName = "Tribal Allies"),
    Territory       UMETA(DisplayName = "Territory Access")
};

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bOptional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TargetRadius;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , ObjectiveType(EQuest_ObjectiveType::Hunt)
        , TargetTag(TEXT(""))
        , RequiredCount(1)
        , CurrentCount(0)
        , bCompleted(false)
        , bOptional(false)
        , TargetLocation(FVector::ZeroVector)
        , TargetRadius(500.0f)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Reward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_RewardType RewardType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ResourceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText RewardDescription;

    FQuest_Reward()
        : RewardType(EQuest_RewardType::Resources)
        , ResourceID(TEXT(""))
        , Quantity(1)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Data
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Priority Priority;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    EQuest_Status Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Reward> Rewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestGiverNPCID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> PrerequisiteQuestIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    float ElapsedTime;

    FQuest_Data()
        : QuestID(TEXT(""))
        , Priority(EQuest_Priority::Normal)
        , Status(EQuest_Status::Inactive)
        , QuestGiverNPCID(TEXT(""))
        , bRepeatable(false)
        , TimeLimit(0.0f)
        , ElapsedTime(0.0f)
    {}
};
