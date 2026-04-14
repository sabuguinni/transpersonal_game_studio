#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "QuestManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStatusChanged, const FString&, QuestID, EQuest_QuestStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnObjectiveCompleted, const FString&, QuestID, const FString&, ObjectiveID, bool, bQuestCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEmotionalStateChanged, EQuest_EmotionalState, OldState, EQuest_EmotionalState, NewState);

/**
 * Core Quest Management System
 * Handles quest creation, progression, completion and emotional journey tracking
 * Designed for transpersonal gaming experiences with spiritual growth mechanics
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
    bool CreateQuest(const FQuest_QuestData& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_QuestData GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetCompletedQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetAvailableQuests() const;

    // Objective Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsObjectiveCompleted(const FString& QuestID, const FString& ObjectiveID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    float GetQuestProgress(const FString& QuestID) const;

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
    void CheckQuestPrerequisites();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool ArePrerequisitesMet(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UnlockQuest(const FString& QuestID);

    // Spiritual Growth Integration
    UFUNCTION(BlueprintCallable, Category = "Spiritual Growth")
    void AwardSpiritualGrowth(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Spiritual Growth")
    float GetTotalSpiritualGrowth() const;

    // Quest Journal System
    UFUNCTION(BlueprintCallable, Category = "Quest Journal")
    void AddJournalEntry(const FString& QuestID, const FText& Entry);

    UFUNCTION(BlueprintCallable, Category = "Quest Journal")
    TArray<FText> GetJournalEntries(const FString& QuestID) const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnQuestStatusChanged OnQuestStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnObjectiveCompleted OnObjectiveCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnEmotionalStateChanged OnEmotionalStateChanged;

protected:
    // Quest Storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Data")
    TMap<FString, FQuest_QuestData> AllQuests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Data")
    TArray<FString> ActiveQuestIDs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Data")
    TArray<FString> CompletedQuestIDs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Data")
    TArray<FString> AvailableQuestIDs;

    // Journal System
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Journal")
    TMap<FString, TArray<FText>> QuestJournals;

    // Spiritual Growth Tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spiritual Growth")
    float TotalSpiritualGrowth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spiritual Growth")
    TMap<FString, float> QuestSpiritualRewards;

private:
    void InitializeDefaultQuests();
    void BroadcastQuestStatusChange(const FString& QuestID, EQuest_QuestStatus NewStatus);
    void BroadcastObjectiveCompletion(const FString& QuestID, const FString& ObjectiveID, bool bQuestCompleted);
    void BroadcastEmotionalStateChange(EQuest_EmotionalState OldState, EQuest_EmotionalState NewState);
    bool ValidateQuestData(const FQuest_QuestData& QuestData) const;
    void ProcessQuestCompletion(const FString& QuestID);
    void ProcessEmotionalJourney(const FString& QuestID);
};