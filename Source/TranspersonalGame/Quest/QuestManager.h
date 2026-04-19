#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "QuestManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStatusChanged, const FString&, QuestID, EQuest_QuestStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveCompleted, const FString&, QuestID, const FString&, ObjectiveID);

/**
 * Quest Manager - Core system for managing survival-based quests
 * Handles quest progression, objective tracking, and NPC interactions
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
    EQuest_QuestStatus GetQuestStatus(const FString& QuestID) const;

    // Objective Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    int32 GetObjectiveProgress(const FString& QuestID, const FString& ObjectiveID) const;

    // Quest Data Access
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_QuestData GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetAvailableQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetCompletedQuests() const;

    // Quest Registration
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void RegisterQuest(const FQuest_QuestData& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UnregisterQuest(const FString& QuestID);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnQuestStatusChanged OnQuestStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnObjectiveCompleted OnObjectiveCompleted;

    // Debug Functions
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest Debug")
    void CreateTestQuests();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest Debug")
    void LogAllQuests();

protected:
    // Quest storage
    UPROPERTY(BlueprintReadOnly, Category = "Quest Data")
    TMap<FString, FQuest_QuestData> RegisteredQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Data")
    TArray<FString> ActiveQuestIDs;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Data")
    TArray<FString> CompletedQuestIDs;

    // Internal functions
    void InitializeDefaultQuests();
    bool ValidateQuestPrerequisites(const FString& QuestID) const;
    void NotifyQuestStatusChange(const FString& QuestID, EQuest_QuestStatus NewStatus);
    void CheckQuestCompletion(const FString& QuestID);
    FQuest_QuestData CreateHuntingQuest() const;
    FQuest_QuestData CreateGatheringQuest() const;
    FQuest_QuestData CreateExplorationQuest() const;
    FQuest_QuestData CreateSurvivalQuest() const;
};