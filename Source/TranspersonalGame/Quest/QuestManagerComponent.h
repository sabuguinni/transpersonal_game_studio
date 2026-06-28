#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Quest/QuestTypes.h"
#include "QuestManagerComponent.generated.h"

// ============================================================
// Quest Manager Component
// Attach to GameState or PlayerController to manage all quests
// Agent #14 — Quest & Mission Designer
// Cycle: PROD_CYCLE_AUTO_20260628_005
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestActivated, const FString&, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, const FString&, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestFailed, const FString&, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveUpdated, const FString&, QuestID, const FString&, ObjectiveID);

UCLASS(ClassGroup = (Quest), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuestManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuestManagerComponent();

    // ---- Delegates ----
    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestActivated OnQuestActivated;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestCompleted OnQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestFailed OnQuestFailed;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnObjectiveUpdated OnObjectiveUpdated;

    // ---- Active Quest Registry ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
    TArray<FQuest_Data> ActiveQuests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
    TArray<FQuest_Data> CompletedQuests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
    TArray<FQuest_Data> FailedQuests;

    // ---- Quest Library (all available quests) ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Library")
    TArray<FQuest_Data> QuestLibrary;

    // ---- Core Functions ----
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ActivateQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AbandonQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 ProgressDelta);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void FailQuest(const FString& QuestID);

    // ---- Query Functions ----
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    bool GetQuestData(const FString& QuestID, FQuest_Data& OutQuestData) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    TArray<FQuest_Data> GetActiveQuestsByCategory(EQuest_Category Category) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    int32 GetActiveQuestCount() const;

    // ---- Predefined Quest Builders ----
    UFUNCTION(BlueprintCallable, Category = "Quest|Builder")
    void RegisterDefaultQuests();

    UFUNCTION(BlueprintCallable, Category = "Quest|Builder")
    FQuest_Data BuildQuest_HunterRescue();

    UFUNCTION(BlueprintCallable, Category = "Quest|Builder")
    FQuest_Data BuildQuest_RiverCrossing();

    UFUNCTION(BlueprintCallable, Category = "Quest|Builder")
    FQuest_Data BuildQuest_StampedeWarning();

    UFUNCTION(BlueprintCallable, Category = "Quest|Builder")
    FQuest_Data BuildQuest_FirstHunt();

    UFUNCTION(BlueprintCallable, Category = "Quest|Builder")
    FQuest_Data BuildQuest_CampDefense();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // Check if all mandatory objectives are met and auto-complete quest
    void CheckQuestCompletion(const FString& QuestID);

    // Apply rewards when quest completes
    void ApplyQuestReward(const FQuest_Data& Quest);

    // Time tracking for timed quests
    TMap<FString, float> QuestTimers;

    // Find quest index in ActiveQuests array
    int32 FindActiveQuestIndex(const FString& QuestID) const;
};
