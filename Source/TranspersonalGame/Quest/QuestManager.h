#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuestSystemTypes.h"
#include "QuestManager.generated.h"

// ============================================================
// QuestManager — Agent #14 Quest & Mission Designer
// Cycle: PROD_CYCLE_AUTO_20260629_005
// Central quest tracking and objective evaluation system
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestActivated, FName, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestCompleted, FName, QuestID, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveUpdated, FName, QuestID, int32, ObjectiveIndex);

UCLASS(ClassGroup = (Quest), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuestManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuestManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ---- Quest Lifecycle ----

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ActivateQuest(FName QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AbandonQuest(FName QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestActive(FName QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestCompleted(FName QuestID) const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    EQuest_Status GetQuestStatus(FName QuestID) const;

    // ---- Objective Tracking ----

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ReportKill(FName DinosaurTag);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ReportCollection(FName ResourceTag, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ReportLocationReached(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ReportCraft(FName RecipeTag);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ReportDestruction(FName TargetTag);

    // ---- Quest Data ----

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuest(const FQuest_Data& QuestData);

    UFUNCTION(BlueprintPure, Category = "Quest")
    bool GetQuestData(FName QuestID, FQuest_Data& OutData) const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    TArray<FName> GetActiveQuestIDs() const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    int32 GetActiveQuestCount() const;

    // ---- Built-in Quests ----

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterDefaultQuests();

    // ---- Events ----

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestActivated OnQuestActivated;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestCompleted OnQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnObjectiveUpdated OnObjectiveUpdated;

private:
    UPROPERTY()
    TMap<FName, FQuest_Data> QuestRegistry;

    void EvaluateObjective(FQuest_Data& Quest, int32 ObjIndex);
    void CheckQuestCompletion(FQuest_Data& Quest);
    void GrantReward(const FQuest_Reward& Reward);
    void TickTimeLimitedQuests(float DeltaTime);

    // Predefined quest builders
    FQuest_Data BuildQuest_RaptorNest() const;
    FQuest_Data BuildQuest_HerdMigration() const;
    FQuest_Data BuildQuest_CampDefense() const;
    FQuest_Data BuildQuest_FirstHunt() const;
    FQuest_Data BuildQuest_WaterSource() const;
};
