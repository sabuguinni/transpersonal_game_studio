#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "QuestManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStatusChanged, const FString&, QuestID, EQuest_QuestStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveCompleted, const FString&, QuestID, const FString&, ObjectiveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEmotionalStateChanged, EQuest_EmotionalState, NewState);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestManager();

protected:
    virtual void BeginPlay() override;

    // Quest Database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_QuestData> ActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_QuestData> CompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_QuestData> AvailableQuests;

    // Player State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player State")
    EQuest_EmotionalState CurrentEmotionalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player State")
    float SurvivalExperience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player State")
    TArray<FString> UnlockedQuestLines;

    // Quest Events
    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnQuestStatusChanged OnQuestStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnObjectiveCompleted OnObjectiveCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnEmotionalStateChanged OnEmotionalStateChanged;

public:
    // Quest Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress);

    // Quest Queries
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_QuestData GetQuestData(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetActiveQuests();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetAvailableQuests();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestActive(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestCompleted(const FString& QuestID);

    // Emotional Journey
    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    void UpdateEmotionalState(EQuest_EmotionalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    void ProgressEmotionalJourney(const FString& QuestID, float ProgressAmount);

    UFUNCTION(BlueprintCallable, Category = "Emotional System")
    EQuest_EmotionalState GetCurrentEmotionalState() const;

    // Quest Creation (for dynamic quests)
    UFUNCTION(BlueprintCallable, Category = "Quest Creation")
    void CreateSurvivalQuest(const FString& QuestID, const FText& Title, const FText& Description, 
                           const FVector& TargetLocation, EQuest_ObjectiveType ObjectiveType);

    UFUNCTION(BlueprintCallable, Category = "Quest Creation")
    void CreateHuntingQuest(const FString& QuestID, const FString& DinosaurSpecies, 
                          const FVector& HuntingGrounds, int32 RequiredKills);

    UFUNCTION(BlueprintCallable, Category = "Quest Creation")
    void CreateExplorationQuest(const FString& QuestID, const FVector& ExplorationTarget, 
                              float ExplorationRadius, const FString& DiscoveryType);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void InitializeDefaultQuests();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void SaveQuestProgress();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void LoadQuestProgress();

private:
    // Internal quest management
    FQuest_QuestData* FindQuestByID(const FString& QuestID);
    FQuest_ObjectiveData* FindObjectiveByID(FQuest_QuestData& Quest, const FString& ObjectiveID);
    void CheckQuestCompletion(FQuest_QuestData& Quest);
    void UnlockDependentQuests(const FString& CompletedQuestID);
    void UpdateEmotionalProgress(FQuest_QuestData& Quest);
};