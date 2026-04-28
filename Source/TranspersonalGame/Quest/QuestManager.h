#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "QuestManager.generated.h"

class AQuest_QuestGiver;
class AQuest_ObjectiveMarker;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStatusChanged, const FString&, QuestID, EQuest_QuestStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnObjectiveUpdated, const FString&, QuestID, const FString&, ObjectiveID, int32, NewProgress);

/**
 * Core Quest Management System for Survival Gameplay
 * Handles quest progression, objective tracking, and NPC interactions
 * Focus: Realistic survival quests - hunting, gathering, exploration, crafting
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuestManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuestManager();

    // Subsystem Interface
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
    TArray<FQuest_QuestData> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetCompletedQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_QuestData GetQuestData(const FString& QuestID) const;

    // Objective Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 ProgressAmount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    int32 GetObjectiveProgress(const FString& QuestID, const FString& ObjectiveID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsObjectiveCompleted(const FString& QuestID, const FString& ObjectiveID) const;

    // Quest Discovery and Availability
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FString> GetAvailableQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CanStartQuest(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void RegisterQuestGiver(AQuest_QuestGiver* QuestGiver);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UnregisterQuestGiver(AQuest_QuestGiver* QuestGiver);

    // Location and Interaction Tracking
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnPlayerReachedLocation(const FVector& Location, float Radius = 100.0f);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnPlayerInteractedWithActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnPlayerKilledActor(AActor* KilledActor);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnPlayerCollectedItem(const FString& ItemID, int32 Quantity = 1);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Quest System")
    FOnQuestStatusChanged OnQuestStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Quest System")
    FOnObjectiveUpdated OnObjectiveUpdated;

    // Debug and Development
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest System")
    void CreateDefaultSurvivalQuests();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest System")
    void DebugPrintActiveQuests();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest System")
    void ResetAllQuests();

protected:
    // Quest Storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Data")
    TMap<FString, FQuest_QuestData> AllQuests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Data")
    TArray<FString> ActiveQuestIDs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Data")
    TArray<FString> CompletedQuestIDs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Data")
    TArray<FString> FailedQuestIDs;

    // Quest Givers and Markers
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest NPCs")
    TArray<AQuest_QuestGiver*> RegisteredQuestGivers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Tracking")
    TMap<FString, FVector> QuestLocationMarkers;

    // Internal Methods
    void InitializeDefaultQuests();
    bool CheckQuestPrerequisites(const FString& QuestID) const;
    void ProcessQuestCompletion(const FString& QuestID);
    void UpdateQuestObjectives(const FString& QuestID);
    bool AreAllObjectivesCompleted(const FString& QuestID) const;
    void GiveQuestRewards(const FQuest_QuestData& QuestData);

    // Location Tracking
    void CheckLocationObjectives(const FVector& PlayerLocation);
    float CalculateDistanceToLocation(const FVector& Location1, const FVector& Location2) const;
};