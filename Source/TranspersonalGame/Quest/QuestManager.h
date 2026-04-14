#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "QuestManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStatusChanged, const FString&, QuestID, EQuest_QuestStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnObjectiveUpdated, const FString&, QuestID, const FString&, ObjectiveID, int32, NewProgress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEmotionalStateChanged, EQuest_EmotionalState, OldState, EQuest_EmotionalState, NewState);

/**
 * Core Quest Management System
 * Handles quest progression, emotional journeys, and spiritual growth
 * Designed around the principle that every quest is an emotional arc
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
    EQuest_QuestStatus GetQuestStatus(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetAvailableQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_QuestData GetQuestData(const FString& QuestID) const;

    // Objective Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsObjectiveCompleted(const FString& QuestID, const FString& ObjectiveID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    int32 GetObjectiveProgress(const FString& QuestID, const FString& ObjectiveID) const;

    // Emotional Journey System
    UFUNCTION(BlueprintCallable, Category = "Emotional Journey")
    void UpdateEmotionalState(const FString& QuestID, EQuest_EmotionalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Emotional Journey")
    EQuest_EmotionalState GetCurrentEmotionalState(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Emotional Journey")
    float GetEmotionalProgress(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Emotional Journey")
    void AddEmotionalMilestone(const FString& QuestID, const FText& Milestone);

    // Quest Discovery and Prerequisites
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void RegisterQuest(const FQuest_QuestData& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CheckQuestPrerequisites(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateQuestAvailability();

    // Spiritual Growth Integration
    UFUNCTION(BlueprintCallable, Category = "Spiritual Growth")
    float CalculateSpiritualGrowth(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Spiritual Growth")
    void ApplyQuestRewards(const FString& QuestID);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnQuestStatusChanged OnQuestStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnObjectiveUpdated OnObjectiveUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnEmotionalStateChanged OnEmotionalStateChanged;

    // Debug and Testing
    UFUNCTION(BlueprintCallable, Category = "Quest System", CallInEditor)
    void CreateTestQuests();

    UFUNCTION(BlueprintCallable, Category = "Quest System", CallInEditor)
    void LogQuestSystemStatus();

protected:
    // Quest Storage
    UPROPERTY(BlueprintReadOnly, Category = "Quest Data")
    TMap<FString, FQuest_QuestData> AllQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Data")
    TArray<FString> ActiveQuestIDs;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Data")
    TArray<FString> CompletedQuestIDs;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Data")
    TArray<FString> AvailableQuestIDs;

    // Current Player State
    UPROPERTY(BlueprintReadOnly, Category = "Player State")
    EQuest_EmotionalState CurrentGlobalEmotionalState;

    UPROPERTY(BlueprintReadOnly, Category = "Player State")
    float TotalSpiritualGrowth;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Config")
    bool bAutoUpdateAvailability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Config")
    float EmotionalStateUpdateThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Config")
    int32 MaxActiveQuests;

private:
    // Internal Helper Functions
    void CheckQuestCompletion(const FString& QuestID);
    void UpdateQuestEmotionalProgress(const FString& QuestID);
    bool AreAllObjectivesCompleted(const FQuest_QuestData& Quest) const;
    void BroadcastQuestStatusChange(const FString& QuestID, EQuest_QuestStatus NewStatus);
    void BroadcastObjectiveUpdate(const FString& QuestID, const FString& ObjectiveID, int32 NewProgress);
    void SaveQuestProgress();
    void LoadQuestProgress();
};