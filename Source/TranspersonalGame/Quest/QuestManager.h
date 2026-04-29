#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "../SharedTypes.h"
#include "QuestManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestStarted, const FString&, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, const FString&, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestFailed, const FString&, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveUpdated, const FString&, QuestID, const FString&, ObjectiveID);

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
    bool UpdateObjective(const FString& QuestID, const FString& ObjectiveID, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetCompletedQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_QuestData GetQuestData(const FString& QuestID) const;

    // Quest Registration
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void RegisterQuest(const FQuest_QuestData& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UnregisterQuest(const FString& QuestID);

    // Survival Quest Helpers
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateSurvivalQuest(const FString& QuestID, const FText& Title, const FText& Description, float TimeLimit = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateHuntingQuest(const FString& QuestID, const FString& TargetDinosaurType, int32 RequiredKills);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateExplorationQuest(const FString& QuestID, const FVector& TargetLocation, float AcceptanceRadius = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateGatheringQuest(const FString& QuestID, const FString& ItemType, int32 RequiredQuantity);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnQuestStarted OnQuestStarted;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnQuestCompleted OnQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnQuestFailed OnQuestFailed;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnObjectiveUpdated OnObjectiveUpdated;

protected:
    // Quest Storage
    UPROPERTY(BlueprintReadOnly, Category = "Quest Data")
    TMap<FString, FQuest_QuestData> AllQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Data")
    TArray<FString> ActiveQuestIDs;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Data")
    TArray<FString> CompletedQuestIDs;

    // Helper functions
    void InitializeDefaultQuests();
    bool ArePrerequisitesMet(const FQuest_QuestData& QuestData) const;
    void ProcessQuestCompletion(const FString& QuestID);
    FString GenerateObjectiveID() const;
};