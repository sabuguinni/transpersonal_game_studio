#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "QuestManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStatusChanged, const FString&, QuestID, EQuest_QuestStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveUpdated, const FString&, QuestID, const FString&, ObjectiveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEmotionalStateChanged, EQuest_EmotionalState, OldState, EQuest_EmotionalState, NewState);

/**
 * Quest Manager - Core system for handling survival-focused quests and missions
 * Converts narrative beats into playable survival scenarios with emotional depth
 * Based on realistic prehistoric survival challenges
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
    bool UpdateObjective(const FString& QuestID, const FString& ObjectiveID, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    // Quest Queries
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest System")
    TArray<FQuest_QuestData> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest System")
    TArray<FQuest_QuestData> GetAvailableQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest System")
    FQuest_QuestData GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest System")
    EQuest_QuestStatus GetQuestStatus(const FString& QuestID) const;

    // Emotional Journey System
    UFUNCTION(BlueprintCallable, Category = "Emotional Journey")
    void UpdateEmotionalState(EQuest_EmotionalState NewState);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Emotional Journey")
    EQuest_EmotionalState GetCurrentEmotionalState() const;

    UFUNCTION(BlueprintCallable, Category = "Emotional Journey")
    void ProgressEmotionalJourney(const FString& QuestID, float ProgressAmount);

    // Survival Quest Creation
    UFUNCTION(BlueprintCallable, Category = "Quest Creation")
    FString CreateHuntingQuest(const FString& PreyType, const FVector& HuntingGrounds);

    UFUNCTION(BlueprintCallable, Category = "Quest Creation")
    FString CreateGatheringQuest(const FString& ResourceType, const FVector& GatheringArea);

    UFUNCTION(BlueprintCallable, Category = "Quest Creation")
    FString CreateSurvivalQuest(const FString& ThreatType, float TimeLimit);

    UFUNCTION(BlueprintCallable, Category = "Quest Creation")
    FString CreateCraftingQuest(const FString& ItemType, const TArray<FString>& RequiredMaterials);

    UFUNCTION(BlueprintCallable, Category = "Quest Creation")
    FString CreateExplorationQuest(const FVector& TargetLocation, const FString& DiscoveryType);

    // Quest Events
    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnQuestStatusChanged OnQuestStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnObjectiveUpdated OnObjectiveUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnEmotionalStateChanged OnEmotionalStateChanged;

protected:
    // Quest Data Storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Data")
    TMap<FString, FQuest_QuestData> AllQuests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Data")
    TArray<FString> ActiveQuestIDs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Data")
    TArray<FString> CompletedQuestIDs;

    // Emotional State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Emotional Journey")
    EQuest_EmotionalState CurrentEmotionalState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Emotional Journey")
    TMap<FString, FQuest_EmotionalJourney> QuestEmotionalJourneys;

    // Quest Generation
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Generation")
    int32 NextQuestID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Generation")
    TArray<FString> SurvivalQuestTemplates;

private:
    // Internal Methods
    void InitializeDefaultQuests();
    void InitializeSurvivalQuestTemplates();
    FString GenerateUniqueQuestID(const FString& QuestType);
    void BroadcastQuestStatusChange(const FString& QuestID, EQuest_QuestStatus NewStatus);
    void BroadcastObjectiveUpdate(const FString& QuestID, const FString& ObjectiveID);
    void UpdateQuestEmotionalJourney(const FString& QuestID);
    bool ValidateQuestPrerequisites(const FString& QuestID) const;
    void ProcessQuestRewards(const FString& QuestID);
    void HandleQuestFailure(const FString& QuestID);
};