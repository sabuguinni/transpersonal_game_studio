#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Quest/QuestSystemTypes.h"
#include "QuestManager.generated.h"

// ============================================================
// QuestManager — Agent #14 Quest & Mission Designer
// Cycle: PROD_CYCLE_AUTO_20260629_013
// Manages active quests, objective tracking, and rewards
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuest_OnQuestStarted, const FString&, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuest_OnQuestCompleted, const FString&, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQuest_OnQuestFailed, const FString&, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FQuest_OnObjectiveUpdated, const FString&, QuestID, const FString&, ObjectiveID);

UCLASS(ClassGroup = (Quest), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API AQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Quest Lifecycle ──────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AbandonQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    // ── Objective Tracking ───────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ReportObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ReportHuntKill(const FString& TargetTag);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ReportResourceGathered(const FString& ResourceTag, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ReportLocationReached(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ReportItemCrafted(const FString& ItemTag);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ReportNPCRescued(const FString& NPCID);

    // ── Query ────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    TArray<FQuest_Data> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    TArray<FQuest_Data> GetCompletedQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    FQuest_Data GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    int32 GetActiveQuestCount() const;

    // ── Registration ─────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuest(const FQuest_Data& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuestLibrary(const TArray<FQuest_Data>& Quests);

    // ── Delegates ────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FQuest_OnQuestStarted OnQuestStarted;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FQuest_OnQuestCompleted OnQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FQuest_OnQuestFailed OnQuestFailed;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FQuest_OnObjectiveUpdated OnObjectiveUpdated;

    // ── Config ───────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    int32 MaxActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    bool bEnableTimedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    float LocationReachRadius;

private:
    UPROPERTY()
    TMap<FString, FQuest_Data> QuestLibrary;

    UPROPERTY()
    TArray<FString> ActiveQuestIDs;

    UPROPERTY()
    TArray<FString> CompletedQuestIDs;

    UPROPERTY()
    TArray<FString> FailedQuestIDs;

    void CheckObjectiveCompletion(FQuest_Data& Quest);
    void CheckAllObjectivesComplete(FQuest_Data& Quest);
    void GrantRewards(const FQuest_Data& Quest);
    void UpdateTimedQuests(float DeltaTime);

    bool ArePrerequisitesMet(const FQuest_Data& Quest) const;
};
