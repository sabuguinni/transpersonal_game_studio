#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Quest/QuestSystemTypes.h"
#include "QuestManagerComponent.generated.h"

// ============================================================
// QuestManagerComponent — Agent #14 Quest & Mission Designer
// Cycle: PROD_CYCLE_AUTO_20260629_011
//
// Attached to the PlayerController or GameMode.
// Manages the full lifecycle of quests: available → active →
// completed/failed. Tracks objectives, rewards, and state.
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuest_OnQuestActivated, const FString&, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FQuest_OnObjectiveUpdated, const FString&, QuestID, const FString&, ObjectiveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuest_OnQuestCompleted, const FString&, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuest_OnQuestFailed, const FString&, QuestID);

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuestManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuestManagerComponent();

    // ── Lifecycle ──────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── Quest Registration ─────────────────────────────────
    /** Register a quest definition into the available pool */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuest(const FQuest_Definition& QuestDef);

    /** Load all built-in quests (called on BeginPlay) */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void LoadBuiltInQuests();

    // ── Quest Activation ───────────────────────────────────
    /** Attempt to activate a quest by ID. Returns true if successful. */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ActivateQuest(const FString& QuestID);

    /** Abandon an active quest */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void AbandonQuest(const FString& QuestID);

    // ── Objective Progress ─────────────────────────────────
    /** Report progress on an objective (e.g., killed a raptor, gathered stone) */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ReportObjectiveProgress(const FString& TargetTag, int32 Count = 1);

    /** Directly complete an objective by ID */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    // ── Query ──────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    EQuest_Status GetQuestStatus(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    TArray<FQuest_RuntimeState> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    int32 GetObjectiveProgress(const FString& QuestID, const FString& ObjectiveID) const;

    // ── Delegates ─────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FQuest_OnQuestActivated OnQuestActivated;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FQuest_OnObjectiveUpdated OnObjectiveUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FQuest_OnQuestCompleted OnQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FQuest_OnQuestFailed OnQuestFailed;

    // ── Data ───────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest", meta = (AllowPrivateAccess = "true"))
    TArray<FQuest_Definition> QuestLibrary;

    UPROPERTY(BlueprintReadOnly, Category = "Quest", meta = (AllowPrivateAccess = "true"))
    TArray<FQuest_RuntimeState> ActiveQuestStates;

    UPROPERTY(BlueprintReadOnly, Category = "Quest", meta = (AllowPrivateAccess = "true"))
    TArray<FString> CompletedQuestIDs;

private:
    /** Check if all objectives in a quest are done and trigger completion */
    void CheckQuestCompletion(const FString& QuestID);

    /** Grant rewards for a completed quest */
    void GrantRewards(const FQuest_Definition& QuestDef);

    /** Find quest definition by ID */
    const FQuest_Definition* FindQuestDefinition(const FString& QuestID) const;

    /** Find runtime state by ID */
    FQuest_RuntimeState* FindRuntimeState(const FString& QuestID);
};
