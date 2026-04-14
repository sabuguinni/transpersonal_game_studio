#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "../SharedTypes.h"
#include "QuestManager.generated.h"

// Forward declarations
class UQuestComponent;
class AQuestNPC;
class AQuestObjective;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStatusChanged, const FString&, QuestID, EQuest_QuestStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveCompleted, const FString&, QuestID, const FString&, ObjectiveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEmotionalStateChanged, EQuest_EmotionalState, OldState, EQuest_EmotionalState, NewState);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core quest data storage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_QuestData> ActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_QuestData> CompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_QuestData> AvailableQuests;

    // Emotional journey tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Journey")
    EQuest_EmotionalState CurrentEmotionalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Journey")
    float EmotionalGrowthPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional Journey")
    TMap<EQuest_EmotionalState, float> EmotionalMasteryLevels;

    // Quest system configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float QuestUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableEmotionalJourneys;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableQuestChaining;

public:
    // Quest management functions
    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    bool AbandonQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    FQuest_QuestData GetQuestData(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    TArray<FQuest_QuestData> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    TArray<FQuest_QuestData> GetAvailableQuests() const;

    // Objective management
    UFUNCTION(BlueprintCallable, Category = "Objectives")
    bool UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 ProgressIncrement = 1);

    UFUNCTION(BlueprintCallable, Category = "Objectives")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Objectives")
    float GetQuestProgress(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Objectives")
    TArray<FQuest_ObjectiveData> GetQuestObjectives(const FString& QuestID);

    // Emotional journey functions
    UFUNCTION(BlueprintCallable, Category = "Emotional Journey")
    void TriggerEmotionalTransition(EQuest_EmotionalState NewState, float IntensityMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Emotional Journey")
    EQuest_EmotionalState GetCurrentEmotionalState() const;

    UFUNCTION(BlueprintCallable, Category = "Emotional Journey")
    float GetEmotionalMastery(EQuest_EmotionalState EmotionalState) const;

    UFUNCTION(BlueprintCallable, Category = "Emotional Journey")
    void AddEmotionalGrowth(float GrowthPoints);

    // Quest discovery and prerequisites
    UFUNCTION(BlueprintCallable, Category = "Quest Discovery")
    void CheckQuestPrerequisites();

    UFUNCTION(BlueprintCallable, Category = "Quest Discovery")
    bool ArePrerequisitesMet(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Discovery")
    void RegisterQuestNPC(AQuestNPC* QuestNPC);

    UFUNCTION(BlueprintCallable, Category = "Quest Discovery")
    void RegisterQuestObjective(AQuestObjective* QuestObjective);

    // Event delegates
    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnQuestStatusChanged OnQuestStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnObjectiveCompleted OnObjectiveCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnEmotionalStateChanged OnEmotionalStateChanged;

protected:
    // Internal quest processing
    void ProcessActiveQuests();
    void ProcessEmotionalJourneys();
    void CheckQuestTimeouts();
    void UpdateQuestChains();

    // Helper functions
    FQuest_QuestData* FindQuestByID(const FString& QuestID);
    bool IsQuestActive(const FString& QuestID);
    void BroadcastQuestStatusChange(const FString& QuestID, EQuest_QuestStatus NewStatus);
    void BroadcastEmotionalStateChange(EQuest_EmotionalState OldState, EQuest_EmotionalState NewState);

    // Registered actors
    UPROPERTY()
    TArray<AQuestNPC*> RegisteredQuestNPCs;

    UPROPERTY()
    TArray<AQuestObjective*> RegisteredQuestObjectives;

private:
    // Internal timers
    float QuestUpdateTimer;
    float EmotionalJourneyTimer;
};