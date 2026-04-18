#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "QuestManager.generated.h"

class AQuest_QuestGiverNPC;
class UQuest_QuestComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStatusChanged, const FString&, QuestID, EQuest_QuestStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveCompleted, const FString&, QuestID, const FString&, ObjectiveID);

/**
 * Quest Manager - Central system for handling all quest logic
 * Manages quest progression, objectives, and NPC interactions
 */
UCLASS()
class TRANSPERSONALGAME_API UQuestManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuestManager();

    // Subsystem interface
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
    EQuest_QuestStatus GetQuestStatus(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetAvailableQuests() const;

    // Objective Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsObjectiveCompleted(const FString& QuestID, const FString& ObjectiveID) const;

    // Quest Data Access
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_QuestData GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool AddQuest(const FQuest_QuestData& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool RemoveQuest(const FString& QuestID);

    // Location-based objectives
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CheckLocationObjectives(const FVector& PlayerLocation, float Radius = 200.0f);

    // NPC Interaction
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FString> GetQuestsFromNPC(const FString& NPCID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void RegisterQuestGiver(AQuest_QuestGiverNPC* QuestGiver);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UnregisterQuestGiver(AQuest_QuestGiverNPC* QuestGiver);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnQuestStatusChanged OnQuestStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnObjectiveCompleted OnObjectiveCompleted;

    // Debug functions
    UFUNCTION(BlueprintCallable, Category = "Quest Debug", CallInEditor = true)
    void DebugPrintAllQuests();

    UFUNCTION(BlueprintCallable, Category = "Quest Debug", CallInEditor = true)
    void CreateTestQuests();

protected:
    // Quest storage
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Data")
    TMap<FString, FQuest_QuestData> AllQuests;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Data")
    TArray<FString> ActiveQuestIDs;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Data")
    TArray<FString> CompletedQuestIDs;

    // NPC references
    UPROPERTY()
    TArray<AQuest_QuestGiverNPC*> RegisteredQuestGivers;

private:
    // Internal quest logic
    void InitializeDefaultQuests();
    bool CheckQuestPrerequisites(const FString& QuestID) const;
    void UpdateQuestStatus(const FString& QuestID, EQuest_QuestStatus NewStatus);
    void ProcessQuestCompletion(const FString& QuestID);
    void ProcessObjectiveCompletion(const FString& QuestID, const FString& ObjectiveID);

    // Quest validation
    bool IsValidQuestID(const FString& QuestID) const;
    bool IsValidObjectiveID(const FString& QuestID, const FString& ObjectiveID) const;
};