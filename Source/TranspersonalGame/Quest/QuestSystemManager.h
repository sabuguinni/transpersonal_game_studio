#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "QuestSystemManager.generated.h"

UENUM(BlueprintType)
enum class EQuestStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Abandoned       UMETA(DisplayName = "Abandoned")
};

UENUM(BlueprintType)
enum class EQuestType : uint8
{
    MainStory       UMETA(DisplayName = "Main Story"),
    SideQuest       UMETA(DisplayName = "Side Quest"),
    Exploration     UMETA(DisplayName = "Exploration"),
    Survival        UMETA(DisplayName = "Survival"),
    Spiritual       UMETA(DisplayName = "Spiritual Journey"),
    Community       UMETA(DisplayName = "Community Building"),
    Discovery       UMETA(DisplayName = "Discovery")
};

UENUM(BlueprintType)
enum class EObjectiveType : uint8
{
    GoToLocation    UMETA(DisplayName = "Go To Location"),
    CollectItem     UMETA(DisplayName = "Collect Item"),
    TalkToNPC       UMETA(DisplayName = "Talk To NPC"),
    DefeatEnemy     UMETA(DisplayName = "Defeat Enemy"),
    Survive         UMETA(DisplayName = "Survive"),
    Discover        UMETA(DisplayName = "Discover"),
    Meditate        UMETA(DisplayName = "Meditate"),
    BuildStructure  UMETA(DisplayName = "Build Structure"),
    Craft           UMETA(DisplayName = "Craft Item")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuestObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TargetActorID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredQuantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsOptional;

    FQuestObjective()
    {
        ObjectiveID = "";
        Description = FText::GetEmpty();
        ObjectiveType = EObjectiveType::GoToLocation;
        TargetLocation = FVector::ZeroVector;
        TargetActorID = "";
        RequiredQuantity = 1;
        CurrentProgress = 0;
        bIsCompleted = false;
        bIsOptional = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuestReward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ExperiencePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SpiritualEnergy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText SpecialRewardDescription;

    FQuestReward()
    {
        ExperiencePoints = 0;
        SpiritualEnergy = 0;
        SpecialRewardDescription = FText::GetEmpty();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuestData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EQuestType QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EQuestStatus QuestStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FQuestObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Prerequisites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FQuestReward Rewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RecommendedLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EstimatedDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString QuestGiverID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector QuestStartLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText EmotionalTheme;

    FQuestData()
    {
        QuestID = "";
        QuestName = FText::GetEmpty();
        QuestDescription = FText::GetEmpty();
        QuestType = EQuestType::SideQuest;
        QuestStatus = EQuestStatus::NotStarted;
        RecommendedLevel = 1;
        EstimatedDuration = 10.0f;
        QuestGiverID = "";
        QuestStartLocation = FVector::ZeroVector;
        bIsRepeatable = false;
        EmotionalTheme = FText::GetEmpty();
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStatusChanged, const FString&, QuestID, EQuestStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveCompleted, const FString&, QuestID, const FString&, ObjectiveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, const FString&, QuestID);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuestSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuestSystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Quest Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool AbandonQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuestData> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuestData> GetCompletedQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuestData GetQuestData(const FString& QuestID) const;

    // Objective Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsObjectiveCompleted(const FString& QuestID, const FString& ObjectiveID) const;

    // Quest Discovery & Availability
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuestData> GetAvailableQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CanStartQuest(const FString& QuestID) const;

    // Location-based Quest Triggers
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CheckLocationTriggers(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void RegisterQuestLocation(const FString& QuestID, const FVector& Location, float TriggerRadius);

    // Spiritual & Emotional Journey
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateSpiritualProgress(const FString& QuestID, float SpiritualGain);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    float GetTotalSpiritualEnergy() const;

    // Events
    UPROPERTY(BlueprintAssignable)
    FOnQuestStatusChanged OnQuestStatusChanged;

    UPROPERTY(BlueprintAssignable)
    FOnObjectiveCompleted OnObjectiveCompleted;

    UPROPERTY(BlueprintAssignable)
    FOnQuestCompleted OnQuestCompleted;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    class UDataTable* QuestDataTable;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest System")
    TMap<FString, FQuestData> ActiveQuests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest System")
    TMap<FString, FQuestData> CompletedQuests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest System")
    TMap<FString, FVector> QuestLocationTriggers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest System")
    TMap<FString, float> QuestTriggerRadii;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest System")
    float TotalSpiritualEnergy;

private:
    void LoadQuestData();
    bool CheckPrerequisites(const FString& QuestID) const;
    void GiveQuestRewards(const FQuestData& QuestData);
    void UpdateQuestStatus(const FString& QuestID, EQuestStatus NewStatus);
};