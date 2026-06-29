#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Quest/QuestSystemTypes.h"
#include "QuestManager.generated.h"

// Forward declarations
class ATranspersonalCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestActivated, const FQuest_Definition&, Quest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestObjectiveUpdated, FName, QuestID, int32, ObjectiveIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, FName, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestFailed, FName, QuestID);

/**
 * UQuestManager — World Subsystem managing all active quests.
 * Agent #14 — Quest & Mission Designer
 * Handles quest activation, objective tracking, completion, and rewards.
 */
UCLASS()
class TRANSPERSONALGAME_API UQuestManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UQuestManager();

    // UWorldSubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ── Quest Lifecycle ──────────────────────────────────────────────────────

    /** Activate a quest by ID. Returns false if already active or not found. */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ActivateQuest(FName QuestID);

    /** Abandon an active quest. */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AbandonQuest(FName QuestID);

    /** Fail an active quest (e.g. NPC died, time expired). */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(FName QuestID);

    // ── Objective Tracking ───────────────────────────────────────────────────

    /** Report progress on a specific objective (e.g. killed 1 raptor). */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ReportObjectiveProgress(FName QuestID, int32 ObjectiveIndex, int32 Amount = 1);

    /** Directly complete an objective (e.g. reached a location). */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteObjective(FName QuestID, int32 ObjectiveIndex);

    // ── Query ────────────────────────────────────────────────────────────────

    /** Get all currently active quests. */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_Definition> GetActiveQuests() const;

    /** Get a specific quest by ID. Returns false if not found. */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool GetQuestByID(FName QuestID, FQuest_Definition& OutQuest) const;

    /** Check if a quest is currently active. */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestActive(FName QuestID) const;

    /** Check if a quest has been completed. */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestCompleted(FName QuestID) const;

    /** Get the current status of a quest. */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    EQuest_Status GetQuestStatus(FName QuestID) const;

    // ── Registration ─────────────────────────────────────────────────────────

    /** Register a quest definition into the library. Called at startup. */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuest(const FQuest_Definition& QuestDef);

    /** Register all built-in quests (called during Initialize). */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterAllBuiltInQuests();

    // ── Delegates ────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestActivated OnQuestActivated;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestObjectiveUpdated OnQuestObjectiveUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestCompleted OnQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestFailed OnQuestFailed;

    // ── Herd Integration ─────────────────────────────────────────────────────

    /** Called by StampedeController when a stampede begins. */
    UFUNCTION(BlueprintCallable, Category = "Quest|Herd")
    void OnStampedeTriggered(FVector StampedeOrigin, FVector StampedeDirection);

    /** Called when player enters a herd anchor zone. */
    UFUNCTION(BlueprintCallable, Category = "Quest|Herd")
    void OnPlayerEnteredHerdZone(FName HerdSpecies, FVector ZoneLocation);

private:
    /** All registered quest definitions (library). */
    UPROPERTY()
    TMap<FName, FQuest_Definition> QuestLibrary;

    /** Runtime status of all known quests. */
    UPROPERTY()
    TMap<FName, EQuest_Status> QuestStatusMap;

    /** Current progress per objective per quest. Key = QuestID, Value = array of progress counts. */
    TMap<FName, TArray<int32>> ObjectiveProgress;

    /** Check if all objectives in a quest are complete and trigger completion. */
    void CheckQuestCompletion(FName QuestID);

    /** Grant rewards to the player for completing a quest. */
    void GrantRewards(const FQuest_Definition& Quest);

    /** Build the five built-in prehistoric survival quests. */
    void BuildBuiltInQuests();
};
