#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Quest/QuestTypes.h"
#include "QuestObjectiveSystem.generated.h"

// ============================================================
// Quest Objective System — Agent #14 Quest & Mission Designer
// Cycle: PROD_CYCLE_AUTO_20260628_006
// Handles individual quest objectives, triggers, and completion
// ============================================================

// Objective trigger volume — placed in world as invisible zone
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ObjectiveTrigger : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ObjectiveTrigger();

    // The quest this trigger belongs to
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FName QuestID;

    // The objective index within the quest
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ObjectiveIndex;

    // Radius of the trigger zone in cm
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TriggerRadius;

    // Whether this trigger is currently active
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    bool bIsActive;

    // Optional: kill count required (for hunt objectives)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredKillCount;

    // Current kill count
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    int32 CurrentKillCount;

    // Time limit in seconds (0 = no limit)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimitSeconds;

    // Elapsed time since objective became active
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    float ElapsedTime;

    // Activate this objective trigger
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ActivateTrigger();

    // Deactivate this objective trigger
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void DeactivateTrigger();

    // Called when player enters trigger radius
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerEnterZone();

    // Register a kill for hunt objectives
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterKill(FName DinosaurSpecies);

    // Check if objective is complete
    UFUNCTION(BlueprintPure, Category = "Quest")
    bool IsObjectiveComplete() const;

    // Delegate broadcast when objective completes
    UPROPERTY(BlueprintAssignable, Category = "Quest")
    FQuest_ObjectiveCompletedDelegate OnObjectiveCompleted;

    // Delegate broadcast when objective fails (time limit)
    UPROPERTY(BlueprintAssignable, Category = "Quest")
    FQuest_ObjectiveFailedDelegate OnObjectiveFailed;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    // Sphere collision component for proximity detection
    UPROPERTY()
    class USphereComponent* TriggerSphere;

    // Whether objective has been completed
    bool bCompleted;

    // Whether objective has failed
    bool bFailed;
};

// ============================================================
// Quest Manager — central system tracking all active quests
// ============================================================
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_Manager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_Manager();

    // All quest definitions loaded at startup
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Definition> QuestDatabase;

    // Currently active quests (by QuestID)
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TArray<FName> ActiveQuestIDs;

    // Completed quest IDs (persisted across sessions)
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TArray<FName> CompletedQuestIDs;

    // Failed quest IDs
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TArray<FName> FailedQuestIDs;

    // Start a quest by ID
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(FName QuestID);

    // Complete a quest
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteQuest(FName QuestID);

    // Fail a quest
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void FailQuest(FName QuestID);

    // Abandon a quest
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void AbandonQuest(FName QuestID);

    // Advance to next objective in a quest
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void AdvanceQuestObjective(FName QuestID);

    // Get current objective for a quest
    UFUNCTION(BlueprintPure, Category = "Quest")
    FQuest_Objective GetCurrentObjective(FName QuestID) const;

    // Check if a quest is active
    UFUNCTION(BlueprintPure, Category = "Quest")
    bool IsQuestActive(FName QuestID) const;

    // Check if a quest is completed
    UFUNCTION(BlueprintPure, Category = "Quest")
    bool IsQuestCompleted(FName QuestID) const;

    // Get quest definition by ID
    UFUNCTION(BlueprintPure, Category = "Quest")
    bool GetQuestDefinition(FName QuestID, FQuest_Definition& OutDefinition) const;

    // Register all trigger actors in the level
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest")
    void RegisterAllTriggers();

    // Initialize the 3 core quests for MinPlayableMap
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest")
    void InitializeCoreQuests();

    // Delegate: quest started
    UPROPERTY(BlueprintAssignable, Category = "Quest")
    FQuest_StatusChangedDelegate OnQuestStarted;

    // Delegate: quest completed
    UPROPERTY(BlueprintAssignable, Category = "Quest")
    FQuest_StatusChangedDelegate OnQuestCompleted;

    // Delegate: quest failed
    UPROPERTY(BlueprintAssignable, Category = "Quest")
    FQuest_StatusChangedDelegate OnQuestFailed;

protected:
    virtual void BeginPlay() override;

private:
    // Active quest states (QuestID → current objective index)
    TMap<FName, int32> ActiveQuestProgress;

    // Registered trigger actors
    UPROPERTY()
    TArray<AQuest_ObjectiveTrigger*> RegisteredTriggers;

    // Build the 3 core survival quests
    void BuildRaptorGauntletQuest();
    void BuildSurviveStampedeQuest();
    void BuildHuntLonePredatorQuest();
};
