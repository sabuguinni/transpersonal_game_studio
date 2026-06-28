// QuestManager.h
// Agent #14 — Quest & Mission Designer
// PROD_CYCLE_AUTO_20260628_007
// Manages active quests, objective tracking, and quest state transitions

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Quest/QuestTypes.h"
#include "QuestManager.generated.h"

// Forward declarations
class ATranspersonalCharacter;
class UQuestObjectiveComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestStarted, const FQuest_QuestData&, QuestData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestObjectiveUpdated, FName, QuestID, const FQuest_ObjectiveData&, Objective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestCompleted, FName, QuestID, bool, bSuccess);

UCLASS(ClassGroup = "Quest", BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ─── Quest Lifecycle ───────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Quest|Lifecycle")
    bool StartQuest(FName QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest|Lifecycle")
    bool AbandonQuest(FName QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest|Lifecycle")
    bool CompleteQuest(FName QuestID, bool bSuccess);

    // ─── Objective Tracking ────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Quest|Objectives")
    void UpdateObjectiveProgress(FName QuestID, FName ObjectiveID, int32 ProgressDelta);

    UFUNCTION(BlueprintCallable, Category = "Quest|Objectives")
    void MarkObjectiveComplete(FName QuestID, FName ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest|Objectives")
    bool IsObjectiveComplete(FName QuestID, FName ObjectiveID) const;

    // ─── Query ─────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest|Query")
    bool IsQuestActive(FName QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest|Query")
    EQuest_Status GetQuestStatus(FName QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest|Query")
    TArray<FQuest_QuestData> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest|Query")
    int32 GetActiveQuestCount() const;

    // ─── Stampede Integration ──────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Quest|Events")
    void OnStampedeWitnessed(FVector StampedeLocation, FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest|Events")
    void OnPlayerEscapedStampedePath();

    UFUNCTION(BlueprintCallable, Category = "Quest|Events")
    void OnPredatorTracked(FVector PredatorLocation);

    // ─── Delegates ─────────────────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestStarted OnQuestStarted;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestObjectiveUpdated OnQuestObjectiveUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestCompleted OnQuestCompleted;

    // ─── Configuration ─────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    float ObjectiveCheckInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    float StampedeWitnessRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    float PredatorTrackRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    int32 MaxActiveQuests = 5;

    // ─── Quest Library (pre-defined quests loaded at start) ────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Library")
    TArray<FQuest_QuestData> QuestLibrary;

private:
    // Active quest state map: QuestID → QuestData
    UPROPERTY()
    TMap<FName, FQuest_QuestData> ActiveQuests;

    // Completed quest IDs (for tracking history)
    UPROPERTY()
    TArray<FName> CompletedQuestIDs;

    // Objective progress cache: QuestID_ObjectiveID → current progress
    UPROPERTY()
    TMap<FName, int32> ObjectiveProgressCache;

    // Timer for periodic objective checks
    FTimerHandle ObjectiveCheckTimer;

    // Reference to player character
    UPROPERTY()
    ATranspersonalCharacter* PlayerCharacter;

    // Internal helpers
    FQuest_QuestData* FindQuestInLibrary(FName QuestID);
    FQuest_ObjectiveData* FindObjectiveInQuest(FQuest_QuestData& Quest, FName ObjectiveID);
    void CheckAllObjectivesForQuest(FName QuestID);
    void PeriodicObjectiveCheck();
    bool AreAllRequiredObjectivesComplete(const FQuest_QuestData& Quest) const;
    FName MakeProgressKey(FName QuestID, FName ObjectiveID) const;

    // Predefined quest builders
    void BuildQuestLibrary();
    FQuest_QuestData BuildQuest_EscapeStampede();
    FQuest_QuestData BuildQuest_TrackThePredator();
    FQuest_QuestData BuildQuest_RescueMissingHunters();
    FQuest_QuestData BuildQuest_SalvageSupplies();
};
