#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "QuestSystemManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStatusChanged, const FString&, QuestID, EQuest_QuestStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveCompleted, const FString&, QuestID, const FString&, ObjectiveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEmotionalStateChanged, EQuest_EmotionalState, OldState, EQuest_EmotionalState, NewState);

/**
 * Core quest system manager that handles all quest logic, emotional journeys, and spiritual progression.
 * This system is designed around the principle that every quest should create an emotional arc
 * that transforms the player's inner state, not just their inventory or stats.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuestSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuestSystemManager();

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
    bool UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsObjectiveCompleted(const FString& QuestID, const FString& ObjectiveID) const;

    // Emotional Journey System
    UFUNCTION(BlueprintCallable, Category = "Emotional Journey")
    void UpdateEmotionalState(const FString& QuestID, EQuest_EmotionalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Emotional Journey")
    EQuest_EmotionalState GetCurrentEmotionalState(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Emotional Journey")
    float GetEmotionalProgress(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Emotional Journey")
    void TriggerEmotionalMilestone(const FString& QuestID, const FText& MilestoneText);

    // Spiritual Growth System
    UFUNCTION(BlueprintCallable, Category = "Spiritual Growth")
    void AddSpiritualGrowth(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Spiritual Growth")
    float GetTotalSpiritualGrowth() const;

    UFUNCTION(BlueprintCallable, Category = "Spiritual Growth")
    int32 GetSpiritualLevel() const;

    // Quest Discovery and Prerequisites
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CheckQuestPrerequisites();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool ArePrerequisitesMet(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void RegisterQuestGiver(AActor* QuestGiver, const FString& QuestID);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnQuestStatusChanged OnQuestStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnObjectiveCompleted OnObjectiveCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnEmotionalStateChanged OnEmotionalStateChanged;

    // Debug and Development
    UFUNCTION(BlueprintCallable, Category = "Quest System", CallInEditor)
    void DebugPrintActiveQuests();

    UFUNCTION(BlueprintCallable, Category = "Quest System", CallInEditor)
    void LoadQuestDatabase();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateTestQuests();

protected:
    // Core quest storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Data")
    TMap<FString, FQuest_QuestData> QuestDatabase;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Data")
    TArray<FString> ActiveQuestIDs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Data")
    TArray<FString> CompletedQuestIDs;

    // Spiritual progression
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spiritual Growth")
    float TotalSpiritualGrowth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spiritual Growth")
    int32 CurrentSpiritualLevel;

    // Quest givers and world integration
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Integration")
    TMap<FString, TSoftObjectPtr<AActor>> QuestGivers;

    // Internal methods
    void InitializeQuestDatabase();
    void ProcessQuestCompletion(const FString& QuestID);
    void CalculateEmotionalProgress(FQuest_QuestData& QuestData);
    bool ValidateQuestData(const FQuest_QuestData& QuestData) const;
    void SaveQuestProgress();
    void LoadQuestProgress();

private:
    // Timer for periodic quest updates
    FTimerHandle QuestUpdateTimer;
    void UpdateQuestStates();
};