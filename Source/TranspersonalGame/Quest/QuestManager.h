#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Quest/QuestSystemTypes.h"
#include "QuestManager.generated.h"

// ============================================================
// Quest Manager — Agent #14 Quest & Mission Designer
// Manages all active quests, evaluates objectives each tick,
// broadcasts events on state transitions.
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestStateChanged, const FQuest_Event&, QuestEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveUpdated, const FQuest_Event&, ObjectiveEvent);

UCLASS(ClassGroup = (Quest), meta = (DisplayName = "Quest Manager"))
class TRANSPERSONALGAME_API AQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ─── Quest Registry ───────────────────────────────────────

    /** All quests defined for this level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Registry")
    TArray<FQuest_Definition> QuestRegistry;

    /** IDs of currently active quests */
    UPROPERTY(BlueprintReadOnly, Category = "Quest|Registry")
    TArray<FString> ActiveQuestIDs;

    /** IDs of completed quests (persists across level) */
    UPROPERTY(BlueprintReadOnly, Category = "Quest|Registry")
    TArray<FString> CompletedQuestIDs;

    // ─── Events ───────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestStateChanged OnQuestStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnObjectiveUpdated OnObjectiveUpdated;

    // ─── Public API ───────────────────────────────────────────

    /** Activate a quest by ID */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ActivateQuest(const FString& QuestID);

    /** Mark a quest as failed */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    /** Manually complete a quest (for scripted sequences) */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    /** Increment counter for a CollectResource or KillTarget objective */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void NotifyObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Amount = 1);

    /** Called when player enters a trigger volume (by actor label) */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void NotifyTriggerEntered(const FString& TriggerActorLabel);

    /** Called when player reaches a location (checks ReachLocation objectives) */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void NotifyPlayerLocation(const FVector& PlayerLocation);

    /** Get quest by ID (returns nullptr if not found) */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_Definition GetQuestByID(const FString& QuestID, bool& bFound) const;

    /** Is a specific quest active? */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    /** Is a specific quest completed? */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestCompleted(const FString& QuestID) const;

    // ─── Built-in Quest Definitions ───────────────────────────

    /** Initialize the 3 built-in quests for MinPlayableMap */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest|Setup")
    void InitializeBuiltInQuests();

private:
    /** Evaluate all active quest objectives each tick */
    void TickActiveQuests(float DeltaTime);

    /** Evaluate a single objective */
    bool EvaluateObjective(FQuest_Objective& Objective, float DeltaTime, const FVector& PlayerLocation);

    /** Check if all objectives in a quest are complete */
    bool AreAllObjectivesComplete(const FQuest_Definition& Quest) const;

    /** Broadcast a quest event */
    void BroadcastQuestEvent(const FString& QuestID, const FString& ObjectiveID, EQuest_State NewState, const FString& Description);

    /** Find quest index by ID */
    int32 FindQuestIndex(const FString& QuestID) const;

    /** Cached player pawn location (updated each tick) */
    FVector CachedPlayerLocation;

    /** Tick accumulator for objective evaluation (evaluate every 0.2s) */
    float ObjectiveTickAccumulator;

    static constexpr float ObjectiveTickInterval = 0.2f;
};
