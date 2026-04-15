#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "QuestManager.generated.h"

class ATranspersonalCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStatusChanged, const FString&, QuestID, EQuest_QuestStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnObjectiveCompleted, const FString&, QuestID, const FString&, ObjectiveID, int32, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEmotionalStateChanged, EQuest_EmotionalState, OldState, EQuest_EmotionalState, NewState);

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuestManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

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

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    int32 GetObjectiveProgress(const FString& QuestID, const FString& ObjectiveID) const;

    // Location-based objectives
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CheckLocationObjective(const FString& QuestID, const FString& ObjectiveID, const FVector& PlayerLocation, float Tolerance = 200.0f);

    // Survival-specific quest functions
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool RegisterHuntKill(const FString& DinosaurType, const FVector& KillLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool RegisterResourceGathered(const FString& ResourceType, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool RegisterCraftedItem(const FString& ItemType);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool RegisterAreaExplored(const FVector& Location, float ExplorationRadius);

    // Emotional Journey System
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateEmotionalState(const FString& QuestID, EQuest_EmotionalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    EQuest_EmotionalState GetCurrentEmotionalState(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    float GetEmotionalProgress(const FString& QuestID) const;

    // Quest Creation and Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void RegisterQuest(const FQuest_QuestData& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void InitializeDefaultQuests();

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
    TMap<FString, FQuest_QuestData> ActiveQuests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Data")
    TMap<FString, FQuest_QuestData> CompletedQuests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Data")
    TMap<FString, FQuest_QuestData> AvailableQuests;

    // Player reference for quest tracking
    UPROPERTY()
    ATranspersonalCharacter* PlayerCharacter;

    // Helper functions
    FQuest_QuestData* FindQuestData(const FString& QuestID);
    const FQuest_QuestData* FindQuestData(const FString& QuestID) const;
    FQuest_ObjectiveData* FindObjectiveData(const FString& QuestID, const FString& ObjectiveID);
    const FQuest_ObjectiveData* FindObjectiveData(const FString& QuestID, const FString& ObjectiveID) const;
    
    void CheckQuestCompletion(const FString& QuestID);
    void ProcessQuestRewards(const FQuest_QuestData& QuestData);
    void UpdateEmotionalProgression(const FString& QuestID);

    // Default quest creation helpers
    FQuest_QuestData CreateHuntingQuest();
    FQuest_QuestData CreateGatheringQuest();
    FQuest_QuestData CreateExplorationQuest();
    FQuest_QuestData CreateCraftingQuest();
    FQuest_QuestData CreateSurvivalQuest();
};