// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "QuestTypes.h"
#include "QuestManager.generated.h"

class UQuestInstance;

/**
 * Quest Manager - Core system for managing quest progression and emotional storytelling
 * Inspired by Pawel Sasko's approach to meaningful side quests with complete emotional arcs
 */
UCLASS()
class TRANSPERSONALGAME_API UQuestManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Quest Management
    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    bool AbandonQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    void UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress = 1);

    // Quest Queries
    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    TArray<FString> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    TArray<FString> GetAvailableQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    TArray<FString> GetCompletedQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    EQuestStatus GetQuestStatus(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    UQuestInstance* GetQuestInstance(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    FQuestData GetQuestData(const FString& QuestID) const;

    // Emotional Progression
    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    void TriggerEmotionalMoment(const FString& QuestID, EEmotionalTone Tone, const FString& Context);

    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    TArray<FString> GetQuestsByEmotionalTone(EEmotionalTone Tone) const;

    // Location-based Quest Discovery
    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    void CheckLocationBasedQuests(const FVector& PlayerLocation, float Radius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    void RegisterQuestTrigger(const FString& QuestID, const FVector& Location, float Radius);

    // Dynamic Quest Generation
    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    FString GenerateSurvivalQuest(EEmotionalTone Tone, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    FString GenerateDiscoveryQuest(const FString& DinosaurSpecies, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    FString GenerateEmergencyQuest(const FString& ThreatType, const FVector& Location);

    // Save/Load
    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    void SaveQuestProgress();

    UFUNCTION(BlueprintCallable, Category = "Quest Manager")
    void LoadQuestProgress();

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnQuestStatusChanged OnQuestStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnObjectiveUpdated OnObjectiveUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnQuestCompleted OnQuestCompleted;

protected:
    // Quest Data
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Manager")
    class UDataTable* QuestDataTable;

    UPROPERTY()
    TMap<FString, UQuestInstance*> ActiveQuestInstances;

    UPROPERTY()
    TMap<FString, EQuestStatus> QuestStatusMap;

    // Location Triggers
    UPROPERTY()
    TMap<FString, FVector> QuestTriggerLocations;

    UPROPERTY()
    TMap<FString, float> QuestTriggerRadii;

    // Emotional Tracking
    UPROPERTY()
    TMap<FString, TArray<EEmotionalTone>> QuestEmotionalHistory;

    // Dynamic Quest Counter
    int32 DynamicQuestCounter;

private:
    // Internal Methods
    bool CanStartQuest(const FString& QuestID) const;
    void ProcessQuestCompletion(const FString& QuestID);
    void UpdateQuestAvailability();
    FString GenerateUniqueQuestID(const FString& Prefix);
    void CreateDynamicQuestData(const FString& QuestID, const FQuestData& QuestData);

    // Emotional Design Helpers
    FText GenerateEmotionalDescription(EEmotionalTone Tone, EQuestType Type, const FString& Context);
    TArray<FQuestObjective> GenerateObjectivesForTone(EEmotionalTone Tone, EQuestType Type, const FVector& Location);
};