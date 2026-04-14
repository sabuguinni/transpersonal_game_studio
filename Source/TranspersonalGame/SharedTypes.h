#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "SharedTypes.generated.h"

// Quest System Enums and Structs

UENUM(BlueprintType)
enum class EQuest_QuestType : uint8
{
    MainStory       UMETA(DisplayName = "Main Story"),
    SideQuest       UMETA(DisplayName = "Side Quest"),
    Exploration     UMETA(DisplayName = "Exploration"),
    Survival        UMETA(DisplayName = "Survival"),
    SpiritualJourney UMETA(DisplayName = "Spiritual Journey"),
    CommunityBuilding UMETA(DisplayName = "Community Building"),
    Discovery       UMETA(DisplayName = "Discovery"),
    Meditation      UMETA(DisplayName = "Meditation"),
    Ritual          UMETA(DisplayName = "Ritual")
};

UENUM(BlueprintType)
enum class EQuest_QuestStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    Available       UMETA(DisplayName = "Available"),
    Active          UMETA(DisplayName = "Active"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Locked          UMETA(DisplayName = "Locked")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    GoToLocation    UMETA(DisplayName = "Go To Location"),
    CollectItem     UMETA(DisplayName = "Collect Item"),
    TalkToNPC       UMETA(DisplayName = "Talk To NPC"),
    KillTarget      UMETA(DisplayName = "Kill Target"),
    Meditate        UMETA(DisplayName = "Meditate"),
    Survive         UMETA(DisplayName = "Survive"),
    Discover        UMETA(DisplayName = "Discover"),
    Build           UMETA(DisplayName = "Build"),
    Ritual          UMETA(DisplayName = "Perform Ritual"),
    Observe         UMETA(DisplayName = "Observe"),
    Connect         UMETA(DisplayName = "Connect with Nature")
};

UENUM(BlueprintType)
enum class EQuest_EmotionalState : uint8
{
    Neutral         UMETA(DisplayName = "Neutral"),
    Curious         UMETA(DisplayName = "Curious"),
    Anxious         UMETA(DisplayName = "Anxious"),
    Peaceful        UMETA(DisplayName = "Peaceful"),
    Excited         UMETA(DisplayName = "Excited"),
    Contemplative   UMETA(DisplayName = "Contemplative"),
    Enlightened     UMETA(DisplayName = "Enlightened"),
    Connected       UMETA(DisplayName = "Connected"),
    Transcendent    UMETA(DisplayName = "Transcendent")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ObjectiveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString TargetActorID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredQuantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsOptional;

    FQuest_ObjectiveData()
    {
        ObjectiveID = "";
        Description = FText::GetEmpty();
        Type = EQuest_ObjectiveType::GoToLocation;
        TargetLocation = FVector::ZeroVector;
        TargetActorID = "";
        RequiredQuantity = 1;
        CurrentProgress = 0;
        bIsCompleted = false;
        bIsOptional = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_EmotionalJourney
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Journey")
    EQuest_EmotionalState StartingEmotion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Journey")
    EQuest_EmotionalState TargetEmotion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Journey")
    EQuest_EmotionalState CurrentEmotion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Journey")
    float EmotionalProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Journey")
    TArray<FText> EmotionalMilestones;

    FQuest_EmotionalJourney()
    {
        StartingEmotion = EQuest_EmotionalState::Neutral;
        TargetEmotion = EQuest_EmotionalState::Peaceful;
        CurrentEmotion = EQuest_EmotionalState::Neutral;
        EmotionalProgress = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_QuestData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_QuestType QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_QuestStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_ObjectiveData> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> Prerequisites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_EmotionalJourney EmotionalArc;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float SpiritualGrowthReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    FQuest_QuestData()
    {
        QuestID = "";
        Title = FText::GetEmpty();
        Description = FText::GetEmpty();
        QuestType = EQuest_QuestType::SideQuest;
        Status = EQuest_QuestStatus::NotStarted;
        ExperienceReward = 0;
        SpiritualGrowthReward = 0.0f;
        bIsRepeatable = false;
        TimeLimit = 0.0f;
    }
};