#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "UObject/ObjectMacros.h"
#include "QuestSystemManager.generated.h"

// Forward declarations
class UQuestComponent;
class UObjective;
class AQuestGiver;

UENUM(BlueprintType)
enum class EQuestType : uint8
{
    MainStory       UMETA(DisplayName = "Main Story"),
    SideQuest       UMETA(DisplayName = "Side Quest"),
    Exploration     UMETA(DisplayName = "Exploration"),
    Survival        UMETA(DisplayName = "Survival"),
    SpiritualJourney UMETA(DisplayName = "Spiritual Journey"),
    CommunityBuilding UMETA(DisplayName = "Community Building"),
    Discovery       UMETA(DisplayName = "Discovery"),
    Meditation      UMETA(DisplayName = "Meditation"),
    Crafting        UMETA(DisplayName = "Crafting")
};

UENUM(BlueprintType)
enum class EQuestState : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Available       UMETA(DisplayName = "Available"),
    Active          UMETA(DisplayName = "Active"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Abandoned       UMETA(DisplayName = "Abandoned")
};

UENUM(BlueprintType)
enum class EEmotionalStage : uint8
{
    Curiosity       UMETA(DisplayName = "Curiosity"),
    Challenge       UMETA(DisplayName = "Challenge"),
    Growth          UMETA(DisplayName = "Growth"),
    Resolution      UMETA(DisplayName = "Resolution"),
    Transformation  UMETA(DisplayName = "Transformation")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuestObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsOptional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredProgress;

    FQuestObjective()
    {
        ObjectiveID = TEXT("");
        Description = FText::GetEmpty();
        bIsCompleted = false;
        bIsOptional = false;
        CurrentProgress = 0;
        RequiredProgress = 1;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEmotionalJourney
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Journey")
    EEmotionalStage CurrentStage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Journey")
    float EmotionalIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Journey")
    FText StageDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Journey")
    TArray<FString> UnlockedInsights;

    FEmotionalJourney()
    {
        CurrentStage = EEmotionalStage::Curiosity;
        EmotionalIntensity = 0.0f;
        StageDescription = FText::GetEmpty();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuestData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuestType QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuestState QuestState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuestObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FEmotionalJourney EmotionalArc;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> Prerequisites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float EstimatedDuration;

    FQuestData()
    {
        QuestID = TEXT("");
        QuestTitle = FText::GetEmpty();
        QuestDescription = FText::GetEmpty();
        QuestType = EQuestType::SideQuest;
        QuestState = EQuestState::Inactive;
        ExperienceReward = 0;
        EstimatedDuration = 0.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStateChanged, const FString&, QuestID, EQuestState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveCompleted, const FString&, QuestID, const FString&, ObjectiveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEmotionalStageChanged, const FString&, QuestID, EEmotionalStage, NewStage);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuestSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestSystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Quest Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool AbandonQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    // Quest Queries
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest System")
    TArray<FQuestData> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest System")
    TArray<FQuestData> GetCompletedQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest System")
    FQuestData GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest System")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest System")
    bool IsQuestCompleted(const FString& QuestID) const;

    // Emotional Journey System
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void AdvanceEmotionalStage(const FString& QuestID, EEmotionalStage NewStage, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void AddInsight(const FString& QuestID, const FString& Insight);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest System")
    FEmotionalJourney GetEmotionalJourney(const FString& QuestID) const;

    // Quest Discovery and Prerequisites
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void RegisterQuest(const FQuestData& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CheckQuestPrerequisites();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest System")
    bool ArePrerequisitesMet(const FString& QuestID) const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Quest System")
    FOnQuestStateChanged OnQuestStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Quest System")
    FOnObjectiveCompleted OnObjectiveCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest System")
    FOnEmotionalStageChanged OnEmotionalStageChanged;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest System")
    TMap<FString, FQuestData> AllQuests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest System")
    TArray<FString> ActiveQuestIDs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest System")
    TArray<FString> CompletedQuestIDs;

    // Quest System Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    int32 MaxActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    bool bAutoCheckPrerequisites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    float PrerequisiteCheckInterval;

private:
    void InitializeDefaultQuests();
    void UpdateQuestState(const FString& QuestID, EQuestState NewState);
    bool CheckAllObjectivesCompleted(const FString& QuestID) const;
    
    float LastPrerequisiteCheck;
};