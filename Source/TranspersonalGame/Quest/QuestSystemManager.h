#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "QuestData.h"
#include "QuestObjective.h"
#include "QuestInstance.h"
#include "QuestSystemManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStateChanged, class UQuestInstance*, Quest, EQuestState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveCompleted, class UQuestInstance*, Quest, class UQuestObjective*, Objective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEmotionalJourneyProgressed, FString, JourneyType, float, Progress);

/**
 * Central manager for the quest system in Transpersonal Game
 * Handles quest lifecycle, emotional journey tracking, and narrative integration
 * Designed by Quest & Mission Designer Agent #14
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuestSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UQuestSystemManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Quest Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    class UQuestInstance* StartQuest(const FString& QuestID, AActor* QuestGiver = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool FailQuest(const FString& QuestID, const FString& Reason = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool AbandonQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    class UQuestInstance* GetActiveQuest(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<class UQuestInstance*> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<class UQuestInstance*> GetCompletedQuests() const;

    // Objective Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool FailObjective(const FString& QuestID, const FString& ObjectiveID);

    // Emotional Journey System
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateEmotionalJourney(const FString& JourneyType, float ProgressDelta);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    float GetEmotionalJourneyProgress(const FString& JourneyType) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FString> GetActiveEmotionalJourneys() const;

    // Quest Discovery and Availability
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FString> GetAvailableQuests(AActor* Player = nullptr) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestAvailable(const FString& QuestID, AActor* Player = nullptr) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool HasCompletedQuest(const FString& QuestID) const;

    // Quest Data Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool LoadQuestDatabase(const FString& DatabasePath);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    class UQuestData* GetQuestData(const FString& QuestID) const;

    // Event Delegates
    UPROPERTY(BlueprintAssignable, Category = "Quest System")
    FOnQuestStateChanged OnQuestStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Quest System")
    FOnObjectiveCompleted OnObjectiveCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest System")
    FOnEmotionalJourneyProgressed OnEmotionalJourneyProgressed;

    // Debug and Development
    UFUNCTION(BlueprintCallable, Category = "Quest System", CallInEditor = true)
    void DebugPrintActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System", CallInEditor = true)
    void DebugPrintEmotionalJourneys() const;

protected:
    // Quest Storage
    UPROPERTY()
    TMap<FString, class UQuestData*> QuestDatabase;

    UPROPERTY()
    TMap<FString, class UQuestInstance*> ActiveQuests;

    UPROPERTY()
    TArray<class UQuestInstance*> CompletedQuests;

    UPROPERTY()
    TArray<class UQuestInstance*> FailedQuests;

    // Emotional Journey Tracking
    UPROPERTY()
    TMap<FString, float> EmotionalJourneyProgress;

    // Internal Methods
    void InitializeEmotionalJourneys();
    void ProcessQuestCompletion(class UQuestInstance* Quest);
    void ProcessObjectiveCompletion(class UQuestInstance* Quest, class UQuestObjective* Objective);
    bool CheckQuestPrerequisites(const FString& QuestID, AActor* Player = nullptr) const;
    void UpdateQuestChains(const FString& CompletedQuestID);

    // Quest Chain Management
    UPROPERTY()
    TMap<FString, TArray<FString>> QuestChains;

    // Save/Load Support
    UFUNCTION()
    void SaveQuestProgress();

    UFUNCTION()
    void LoadQuestProgress();

private:
    // Internal state tracking
    bool bIsInitialized;
    FString CurrentQuestDatabasePath;
};