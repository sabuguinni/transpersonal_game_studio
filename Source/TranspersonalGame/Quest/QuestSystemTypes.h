#pragma once

#include "CoreMinimal.h"
#include "QuestSystemTypes.generated.h"

// ============================================================
// Quest System Types — Agent #14 Quest & Mission Designer
// All types prefixed with Quest_ to avoid naming conflicts
// ============================================================

/** Quest completion state */
UENUM(BlueprintType)
enum class EQuest_State : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Active          UMETA(DisplayName = "Active"),
    ObjectiveReached UMETA(DisplayName = "Objective Reached"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed")
};

/** Objective type — what the player must do */
UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    ReachLocation   UMETA(DisplayName = "Reach Location"),
    SurviveDuration UMETA(DisplayName = "Survive Duration"),
    AvoidDetection  UMETA(DisplayName = "Avoid Detection"),
    ObserveTarget   UMETA(DisplayName = "Observe Target"),
    CollectResource UMETA(DisplayName = "Collect Resource"),
    KillTarget      UMETA(DisplayName = "Kill Target"),
    EscapeArea      UMETA(DisplayName = "Escape Area")
};

/** Quest difficulty tier */
UENUM(BlueprintType)
enum class EQuest_Difficulty : uint8
{
    Scavenger   UMETA(DisplayName = "Scavenger"),    // Tutorial tier
    Hunter      UMETA(DisplayName = "Hunter"),       // Standard
    Apex        UMETA(DisplayName = "Apex"),         // Expert
    Extinction  UMETA(DisplayName = "Extinction")   // Hardcore
};

/** Single quest objective */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_State State;

    /** For ReachLocation / EscapeArea objectives */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float AcceptanceRadius;

    /** For SurviveDuration / ObserveTarget objectives (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float RequiredDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float CurrentProgress;

    /** For CollectResource / KillTarget objectives */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    /** Actor label of the associated world marker */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString MarkerActorLabel;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , ObjectiveType(EQuest_ObjectiveType::ReachLocation)
        , State(EQuest_State::Inactive)
        , TargetLocation(FVector::ZeroVector)
        , AcceptanceRadius(200.0f)
        , RequiredDuration(0.0f)
        , CurrentProgress(0.0f)
        , RequiredCount(1)
        , CurrentCount(0)
        , MarkerActorLabel(TEXT(""))
    {}
};

/** Full quest definition */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_Definition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_State QuestState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Difficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    /** NPC actor label that gives this quest */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestGiverLabel;

    /** Reward: resource type and amount */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString RewardResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RewardAmount;

    /** Time limit in seconds (0 = no limit) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimitSeconds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float ElapsedTime;

    /** Whether this quest is triggered by entering a volume */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString TriggerActorLabel;

    FQuest_Definition()
        : QuestID(TEXT(""))
        , QuestName(TEXT(""))
        , QuestDescription(TEXT(""))
        , QuestState(EQuest_State::Inactive)
        , Difficulty(EQuest_Difficulty::Hunter)
        , QuestGiverLabel(TEXT(""))
        , RewardResourceType(TEXT(""))
        , RewardAmount(0)
        , TimeLimitSeconds(0.0f)
        , ElapsedTime(0.0f)
        , TriggerActorLabel(TEXT(""))
    {}
};

/** Quest event notification (broadcast on state change) */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_Event
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_State NewState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString EventDescription;

    FQuest_Event()
        : QuestID(TEXT(""))
        , ObjectiveID(TEXT(""))
        , NewState(EQuest_State::Inactive)
        , EventDescription(TEXT(""))
    {}
};
