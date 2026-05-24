#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "QuestManager.generated.h"

// Forward declarations
class ATranspersonalCharacter;
class UQuestComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStatusChanged, const FString&, QuestID, EQuest_QuestStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveCompleted, const FString&, QuestID, const FString&, ObjectiveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, const FString&, QuestID);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

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

    // Objective Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    int32 GetObjectiveProgress(const FString& QuestID, const FString& ObjectiveID) const;

    // Quest Data Access
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetCompletedQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetAvailableQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_QuestData GetQuestData(const FString& QuestID) const;

    // Survival Quest Specific Functions
    UFUNCTION(BlueprintCallable, Category = "Survival Quests")
    void RegisterKill(const FString& TargetType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Survival Quests")
    void RegisterItemCollected(const FString& ItemType, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Survival Quests")
    void RegisterLocationVisited(const FVector& Location, float Radius = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Survival Quests")
    void RegisterNPCInteraction(const FString& NPCID);

    // Quest Creation and Management
    UFUNCTION(BlueprintCallable, Category = "Quest System", CallInEditor)
    void CreateDefaultSurvivalQuests();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool AddQuest(const FQuest_QuestData& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool RemoveQuest(const FString& QuestID);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnQuestStatusChanged OnQuestStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnObjectiveCompleted OnObjectiveCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnQuestCompleted OnQuestCompleted;

protected:
    // Quest Storage
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Data", meta = (AllowPrivateAccess = "true"))
    TMap<FString, FQuest_QuestData> AllQuests;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Data", meta = (AllowPrivateAccess = "true"))
    TArray<FString> ActiveQuestIDs;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Data", meta = (AllowPrivateAccess = "true"))
    TArray<FString> CompletedQuestIDs;

    // Player Reference
    UPROPERTY(BlueprintReadOnly, Category = "Quest System", meta = (AllowPrivateAccess = "true"))
    ATranspersonalCharacter* PlayerCharacter;

    // Quest Creation Helpers
    FQuest_QuestData CreateHuntingQuest();
    FQuest_QuestData CreateGatheringQuest();
    FQuest_QuestData CreateExplorationQuest();
    FQuest_QuestData CreateSurvivalQuest();

    // Internal Quest Logic
    void CheckQuestCompletion(const FString& QuestID);
    void ProcessQuestRewards(const FQuest_QuestData& QuestData);
    bool ArePrerequisitesMet(const TArray<FString>& Prerequisites) const;
    void UpdateQuestStatus(const FString& QuestID, EQuest_QuestStatus NewStatus);

private:
    // Quest System State
    bool bIsInitialized;
    float LastUpdateTime;
};