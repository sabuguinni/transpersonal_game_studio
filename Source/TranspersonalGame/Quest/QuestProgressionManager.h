#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "QuestTypes.h"
#include "QuestProgressionManager.generated.h"

class UQuestInstance;
class UQuestObjective;
class ATranspersonalCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStateChanged, int32, QuestID, EQuest_QuestState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnObjectiveUpdated, int32, QuestID, int32, ObjectiveID, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, int32, QuestID);

/**
 * Manages dynamic quest progression and integration with survival mechanics
 * Handles quest state transitions, objective tracking, and survival stat integration
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuestProgressionManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestProgressionManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Quest progression tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Progression")
    TArray<UQuestInstance*> ActiveQuests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Progression")
    TArray<UQuestInstance*> CompletedQuests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Progression")
    TArray<UQuestInstance*> FailedQuests;

    // Survival integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Integration")
    float HungerThresholdForQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Integration")
    float ThirstThresholdForQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Integration")
    float FearThresholdForQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Integration")
    float HealthThresholdForQuests;

    // Quest timing and conditions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Timing")
    float QuestCheckInterval;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Timing")
    float LastQuestCheck;

    // Dynamic quest generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Generation")
    int32 MaxActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Generation")
    float QuestGenerationCooldown;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dynamic Generation")
    float LastQuestGeneration;

public:
    // Quest management functions
    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    bool StartQuest(int32 QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    bool CompleteQuest(int32 QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    bool FailQuest(int32 QuestID, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    bool UpdateQuestObjective(int32 QuestID, int32 ObjectiveID, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    EQuest_QuestState GetQuestState(int32 QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    TArray<UQuestInstance*> GetActiveQuests() const { return ActiveQuests; }

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    UQuestInstance* GetQuestByID(int32 QuestID) const;

    // Survival integration functions
    UFUNCTION(BlueprintCallable, Category = "Survival Integration")
    bool CanPlayerAcceptQuest(int32 QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Survival Integration")
    void CheckSurvivalConditions();

    UFUNCTION(BlueprintCallable, Category = "Survival Integration")
    void OnPlayerSurvivalStateChanged(float Health, float Hunger, float Thirst, float Fear);

    // Dynamic quest generation
    UFUNCTION(BlueprintCallable, Category = "Dynamic Generation")
    void GenerateEmergencyQuest(EQuest_QuestType QuestType);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Generation")
    void CheckForDynamicQuestTriggers();

    // Event delegates
    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnQuestStateChanged OnQuestStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnObjectiveUpdated OnObjectiveUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnQuestCompleted OnQuestCompleted;

protected:
    // Internal progression logic
    void UpdateQuestProgression(float DeltaTime);
    void CheckQuestFailureConditions();
    void ProcessQuestObjectives();
    
    // Survival stat monitoring
    ATranspersonalCharacter* GetPlayerCharacter() const;
    bool IsPlayerInDanger() const;
    bool ShouldGenerateEmergencyQuest() const;

    // Quest state management
    void TransitionQuestState(UQuestInstance* Quest, EQuest_QuestState NewState);
    void NotifyQuestStateChange(int32 QuestID, EQuest_QuestState NewState);
    
private:
    // Internal tracking
    UPROPERTY()
    ATranspersonalCharacter* CachedPlayerCharacter;

    UPROPERTY()
    TMap<int32, EQuest_QuestState> QuestStates;

    UPROPERTY()
    TMap<int32, float> QuestTimers;
};