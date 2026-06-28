// QuestManager.h
// Agent #14 — Quest & Mission Designer
// Cycle: PROD_CYCLE_AUTO_20260628_003
// Central quest manager: tracks active quests, evaluates objectives,
// integrates with CrowdStampedeController trigger events.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Quest/QuestSystemTypes.h"
#include "QuestManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestStateChanged, const FString&, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveCompleted, const FString&, QuestID, const FString&, ObjectiveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, const FString&, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestFailed, const FString&, QuestID);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ── Quest Registry ─────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Registry")
    TArray<FQuest_Definition> AllQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Active")
    TArray<FString> ActiveQuestIDs;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Completed")
    TArray<FString> CompletedQuestIDs;

    // ── Quest Control ──────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    FQuest_Definition GetQuestDefinition(const FString& QuestID) const;

    // ── Crowd Event Integration ────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Quest|Events")
    void OnStampedeTriggered(FVector StampedeOrigin);

    UFUNCTION(BlueprintCallable, Category = "Quest|Events")
    void OnPredatorNearCamp(FVector PredatorLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest|Events")
    void OnNPCSeparated(FString NPCName, FVector SeparationLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest|Events")
    void OnPlayerEntersZone(FString ZoneTag, FVector PlayerLocation);

    // ── Objective Proximity Check ──────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Quest|Objectives")
    void CheckProximityObjectives(FVector PlayerLocation);

    // ── Delegates ─────────────────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestStateChanged OnQuestStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnObjectiveCompleted OnObjectiveCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestCompleted OnQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestFailed OnQuestFailed;

    // ── Time Limit Tracking ────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Timer")
    TMap<FString, float> QuestTimers;

private:
    void RegisterDefaultQuests();
    void EvaluateQuestCompletion(const FString& QuestID);
    void GrantReward(const FQuest_Reward& Reward);
    FQuest_Definition* FindQuest(const FString& QuestID);
};
