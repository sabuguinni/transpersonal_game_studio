#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "QuestManager.generated.h"

class AQuest_QuestGiver;
class UQuest_QuestComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestStarted, const FQuest_QuestData&, QuestData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, const FQuest_QuestData&, QuestData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveUpdated, const FString&, QuestID, const FQuest_ObjectiveData&, Objective);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuestManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuestManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Quest management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool UpdateObjective(const FString& QuestID, const FString& ObjectiveID, int32 Progress);

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

    // Objective tracking
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void TrackLocationObjective(const FString& QuestID, const FString& ObjectiveID, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void TrackKillObjective(const FString& QuestID, const FString& ObjectiveID, const FString& TargetType);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void TrackCollectionObjective(const FString& QuestID, const FString& ObjectiveID, const FString& ItemType, int32 Amount);

    // Quest registration
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void RegisterQuest(const FQuest_QuestData& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void RegisterQuestGiver(AQuest_QuestGiver* QuestGiver);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Quest System")
    FOnQuestStarted OnQuestStarted;

    UPROPERTY(BlueprintAssignable, Category = "Quest System")
    FOnQuestCompleted OnQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest System")
    FOnObjectiveUpdated OnObjectiveUpdated;

protected:
    // Quest storage
    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    TMap<FString, FQuest_QuestData> AllQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    TArray<FString> ActiveQuestIDs;

    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    TArray<FString> CompletedQuestIDs;

    UPROPERTY(BlueprintReadOnly, Category = "Quest System")
    TArray<AQuest_QuestGiver*> RegisteredQuestGivers;

    // Helper functions
    void InitializeDefaultQuests();
    void CreateSurvivalQuests();
    void CreateHuntingQuests();
    void CreateExplorationQuests();
    
    bool CheckQuestPrerequisites(const FQuest_QuestData& QuestData) const;
    void UpdateQuestStatus(FQuest_QuestData& QuestData);
    void CheckObjectiveCompletion(FQuest_QuestData& QuestData, FQuest_ObjectiveData& Objective);

private:
    bool bIsInitialized;
    
    // Quest validation
    bool IsValidQuestID(const FString& QuestID) const;
    bool IsValidObjectiveID(const FString& QuestID, const FString& ObjectiveID) const;
};