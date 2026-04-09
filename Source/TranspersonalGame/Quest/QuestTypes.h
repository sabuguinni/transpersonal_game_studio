// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "QuestTypes.generated.h"

UENUM(BlueprintType)
enum class EQuestType : uint8
{
    MainStory       UMETA(DisplayName = "Main Story"),
    SideQuest       UMETA(DisplayName = "Side Quest"),
    Discovery       UMETA(DisplayName = "Discovery"),
    Survival        UMETA(DisplayName = "Survival"),
    Collection      UMETA(DisplayName = "Collection"),
    Exploration     UMETA(DisplayName = "Exploration"),
    Interaction     UMETA(DisplayName = "Interaction"),
    Domestication   UMETA(DisplayName = "Domestication"),
    Emergency       UMETA(DisplayName = "Emergency")
};

UENUM(BlueprintType)
enum class EQuestStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    Available       UMETA(DisplayName = "Available"),
    Active          UMETA(DisplayName = "Active"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Abandoned       UMETA(DisplayName = "Abandoned"),
    OnHold          UMETA(DisplayName = "On Hold")
};

UENUM(BlueprintType)
enum class EObjectiveType : uint8
{
    KillTarget      UMETA(DisplayName = "Kill Target"),
    CollectItem     UMETA(DisplayName = "Collect Item"),
    ReachLocation   UMETA(DisplayName = "Reach Location"),
    TalkToNPC       UMETA(DisplayName = "Talk To NPC"),
    Survive         UMETA(DisplayName = "Survive"),
    Observe         UMETA(DisplayName = "Observe"),
    Build           UMETA(DisplayName = "Build"),
    Craft           UMETA(DisplayName = "Craft"),
    Domesticate     UMETA(DisplayName = "Domesticate"),
    Escape          UMETA(DisplayName = "Escape"),
    Discover        UMETA(DisplayName = "Discover"),
    Protect         UMETA(DisplayName = "Protect")
};

UENUM(BlueprintType)
enum class EEmotionalTone : uint8
{
    Wonder          UMETA(DisplayName = "Wonder"),
    Fear            UMETA(DisplayName = "Fear"),
    Hope            UMETA(DisplayName = "Hope"),
    Desperation     UMETA(DisplayName = "Desperation"),
    Triumph         UMETA(DisplayName = "Triumph"),
    Melancholy      UMETA(DisplayName = "Melancholy"),
    Curiosity       UMETA(DisplayName = "Curiosity"),
    Urgency         UMETA(DisplayName = "Urgency"),
    Peace           UMETA(DisplayName = "Peace"),
    Tension         UMETA(DisplayName = "Tension")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuestObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    EObjectiveType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FGameplayTagContainer RequiredTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 TargetCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsOptional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    bool bIsHidden;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
    float TargetRadius;

    FQuestObjective()
    {
        ObjectiveID = "";
        Description = FText::GetEmpty();
        Type = EObjectiveType::ReachLocation;
        TargetCount = 1;
        CurrentCount = 0;
        bIsOptional = false;
        bIsHidden = false;
        TargetLocation = FVector::ZeroVector;
        TargetRadius = 500.0f;
    }

    bool IsCompleted() const
    {
        return CurrentCount >= TargetCount;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuestReward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    int32 ExperiencePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    TMap<FString, int32> Items;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    FGameplayTagContainer UnlockedFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    FText RewardDescription;

    FQuestReward()
    {
        ExperiencePoints = 0;
        RewardDescription = FText::GetEmpty();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuestData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText Summary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuestType QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EEmotionalTone EmotionalTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuestObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuestReward Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> PrerequisiteQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FGameplayTagContainer RequiredTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestGiverID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector QuestLocation;

    FQuestData()
    {
        QuestID = "";
        Title = FText::GetEmpty();
        Description = FText::GetEmpty();
        Summary = FText::GetEmpty();
        QuestType = EQuestType::SideQuest;
        EmotionalTone = EEmotionalTone::Curiosity;
        Priority = 1;
        TimeLimit = 0.0f;
        bIsRepeatable = false;
        QuestGiverID = "";
        QuestLocation = FVector::ZeroVector;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStatusChanged, const FString&, QuestID, EQuestStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveUpdated, const FString&, QuestID, const FString&, ObjectiveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, const FString&, QuestID);