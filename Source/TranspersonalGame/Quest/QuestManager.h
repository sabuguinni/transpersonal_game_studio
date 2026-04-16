#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "QuestManager.generated.h"

class AQuestGiver;
class AQuestObjective;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStatusChanged, const FString&, QuestID, EQuest_QuestStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnObjectiveUpdated, const FString&, QuestID, const FString&, ObjectiveID, int32, NewProgress);

/**
 * Quest Manager - Core quest system subsystem
 * Manages all active quests, objectives, and quest progression
 * Realistic survival-focused quest system without spiritual content
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuestManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuestManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Quest Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_QuestData GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetAvailableQuests() const;

    // Objective Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 ProgressAmount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsObjectiveCompleted(const FString& QuestID, const FString& ObjectiveID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    int32 GetObjectiveProgress(const FString& QuestID, const FString& ObjectiveID) const;

    // Quest Registration (for quest givers and objectives)
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void RegisterQuestGiver(AQuestGiver* QuestGiver);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void RegisterQuestObjective(AQuestObjective* QuestObjective);

    // Location-based objectives
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CheckLocationObjective(const FString& QuestID, const FString& ObjectiveID, const FVector& PlayerLocation, float Tolerance = 500.0f);

    // Item collection objectives
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CheckItemCollectionObjective(const FString& QuestID, const FString& ObjectiveID, const FString& ItemID, int32 Quantity);

    // Kill target objectives
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CheckKillTargetObjective(const FString& QuestID, const FString& ObjectiveID, const FString& TargetType);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Quest System")
    FOnQuestStatusChanged OnQuestStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Quest System")
    FOnObjectiveUpdated OnObjectiveUpdated;

    // Debug functions
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest System")
    void DebugPrintAllQuests();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest System")
    void LoadDefaultSurvivalQuests();

protected:
    // Quest storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest System")
    TMap<FString, FQuest_QuestData> AllQuests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest System")
    TArray<FString> ActiveQuestIDs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest System")
    TArray<FString> CompletedQuestIDs;

    // Registered quest actors
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest System")
    TArray<AQuestGiver*> RegisteredQuestGivers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest System")
    TArray<AQuestObjective*> RegisteredQuestObjectives;

private:
    // Internal quest management
    void InitializeDefaultQuests();
    void CreateSurvivalQuest(const FString& QuestID, const FText& Title, const FText& Description, EQuest_QuestType Type);
    void AddObjectiveToQuest(const FString& QuestID, const FString& ObjectiveID, const FText& Description, EQuest_ObjectiveType Type, const FVector& Location = FVector::ZeroVector, const FString& TargetID = "", int32 RequiredQuantity = 1);
    
    bool CheckQuestPrerequisites(const FString& QuestID) const;
    void UpdateQuestStatus(const FString& QuestID, EQuest_QuestStatus NewStatus);
    void CheckQuestCompletion(const FString& QuestID);
    void GiveQuestRewards(const FString& QuestID);
};