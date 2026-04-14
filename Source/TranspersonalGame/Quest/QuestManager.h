#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "QuestManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStatusChanged, const FString&, QuestID, EQuest_QuestStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnObjectiveCompleted, const FString&, QuestID, const FString&, ObjectiveID, bool, bQuestCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEmotionalStateChanged, EQuest_EmotionalState, OldState, EQuest_EmotionalState, NewState);

/**
 * Quest Manager Subsystem - Handles all quest logic and progression
 * Manages emotional journeys and spiritual awakening mechanics
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

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_QuestData GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetAvailableQuests() const;

    // Objective Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress);

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

    // Quest Creation and Registration
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool RegisterQuest(const FQuest_QuestData& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool UnregisterQuest(const FString& QuestID);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void SaveQuestProgress();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void LoadQuestProgress();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void ResetAllQuests();

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnQuestStatusChanged OnQuestStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnObjectiveCompleted OnObjectiveCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnEmotionalStateChanged OnEmotionalStateChanged;

protected:
    // Quest Storage
    UPROPERTY(BlueprintReadOnly, Category = "Quest System", meta = (AllowPrivateAccess = "true"))
    TMap<FString, FQuest_QuestData> RegisteredQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Quest System", meta = (AllowPrivateAccess = "true"))
    TMap<FString, EQuest_QuestStatus> QuestStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Quest System", meta = (AllowPrivateAccess = "true"))
    TMap<FString, FQuest_EmotionalJourney> EmotionalJourneys;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Config", meta = (AllowPrivateAccess = "true"))
    bool bAutoSaveProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Config", meta = (AllowPrivateAccess = "true"))
    float AutoSaveInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Config", meta = (AllowPrivateAccess = "true"))
    int32 MaxActiveQuests;

private:
    // Internal helpers
    bool CheckQuestPrerequisites(const FString& QuestID) const;
    void UpdateQuestStatus(const FString& QuestID, EQuest_QuestStatus NewStatus);
    void ProcessQuestCompletion(const FString& QuestID);
    void CalculateEmotionalProgress(const FString& QuestID);
    
    // Auto-save timer
    FTimerHandle AutoSaveTimerHandle;
    void AutoSaveQuestProgress();
};