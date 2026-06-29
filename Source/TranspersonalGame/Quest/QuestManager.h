// QuestManager.h
// Agent #14 — Quest & Mission Designer
// PROD_CYCLE_AUTO_20260629_007
// UGameInstanceSubsystem managing all active quests, objectives, and rewards.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Quest/QuestSystemTypes.h"
#include "QuestManager.generated.h"

// Forward declarations
class UCrowdSimulationManager;
class ATranspersonalCharacter;

// ─── Runtime objective state ────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FQuest_ObjectiveRuntime
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    FName ObjectiveID;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    EQuest_ObjectiveType Type = EQuest_ObjectiveType::Hunt;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    FText Description;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    int32 CurrentCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    int32 RequiredCount = 1;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    bool bCompleted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    float TargetRadius = 500.f;
};

// ─── Runtime quest instance ──────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FQuest_RuntimeInstance
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    FName QuestID;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    FText QuestTitle;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    FText QuestDescription;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    EQuest_Status Status = EQuest_Status::Inactive;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    EQuest_Tier Tier = EQuest_Tier::Survival;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TArray<FQuest_ObjectiveRuntime> Objectives;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    FQuest_Reward Reward;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    float TimeStarted = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    float TimeLimit = 0.f; // 0 = no limit

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    bool bHasTimeLimit = false;
};

// ─── Quest event delegate ────────────────────────────────────────────────────
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStatusChanged, FName, QuestID, EQuest_Status, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveUpdated, FName, QuestID, FName, ObjectiveID);

// ─── Quest Manager Subsystem ─────────────────────────────────────────────────
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UQuestManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ── Quest lifecycle ──────────────────────────────────────────────────────

    /** Start a quest by ID. Returns false if already active or not found. */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(FName QuestID);

    /** Complete an objective within a quest (increment count). */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ProgressObjective(FName QuestID, FName ObjectiveID, int32 Amount = 1);

    /** Force-fail a quest (e.g. NPC died, time ran out). */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(FName QuestID);

    /** Abandon a quest voluntarily. */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AbandonQuest(FName QuestID);

    /** Get current status of a quest. */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    EQuest_Status GetQuestStatus(FName QuestID) const;

    /** Get all active quests. */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_RuntimeInstance> GetActiveQuests() const;

    /** Get a specific quest runtime instance. */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool GetQuestInstance(FName QuestID, FQuest_RuntimeInstance& OutInstance) const;

    // ── Survival quest triggers ──────────────────────────────────────────────

    /** Called when player kills a dinosaur — auto-progresses hunt objectives. */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void NotifyDinosaurKilled(FName DinosaurSpecies);

    /** Called when player collects a resource item. */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void NotifyResourceCollected(FName ResourceType);

    /** Called when player reaches a location — checks reach objectives. */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void NotifyLocationReached(FVector PlayerLocation);

    /** Called when a crowd NPC dies — checks protect objectives. */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void NotifyNPCDied(FName NPCRole);

    /** Called when player crafts an item — checks craft objectives. */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void NotifyItemCrafted(FName ItemName);

    /** Called when player survives N seconds in a danger zone. */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void NotifySurvivalTick(float DeltaSeconds, bool bInDangerZone);

    // ── Events ───────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestStatusChanged OnQuestStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnObjectiveUpdated OnObjectiveUpdated;

    // ── Debug ────────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest|Debug")
    void Debug_StartAllQuests();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest|Debug")
    void Debug_PrintQuestState();

private:
    // Active quest registry
    UPROPERTY()
    TMap<FName, FQuest_RuntimeInstance> ActiveQuests;

    // Survival tick accumulator (for survive-type objectives)
    float SurvivalDangerTimer = 0.f;

    // Internal helpers
    void RegisterBuiltInQuests();
    void CheckQuestCompletion(FName QuestID);
    void GrantReward(const FQuest_Reward& Reward);
    FQuest_RuntimeInstance BuildDefendTribeQuest();
    FQuest_RuntimeInstance BuildRaptorNestQuest();
    FQuest_RuntimeInstance BuildStampedeEscapeQuest();
    FQuest_RuntimeInstance BuildFirstHuntQuest();
    FQuest_RuntimeInstance BuildScoutValleyQuest();
};
