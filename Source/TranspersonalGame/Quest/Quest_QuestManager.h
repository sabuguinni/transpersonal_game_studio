#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "../Core/SharedTypes.h"
#include "Quest_QuestManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_QuestData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_QuestType QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_QuestStatus QuestStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString TargetActorTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector ObjectiveLocation;

    FQuest_QuestData()
    {
        QuestID = TEXT("");
        QuestName = TEXT("");
        QuestDescription = TEXT("");
        QuestType = EQuest_QuestType::Hunt;
        QuestStatus = EQuest_QuestStatus::NotStarted;
        RequiredAmount = 1;
        CurrentProgress = 0;
        TargetActorTag = TEXT("");
        ObjectiveLocation = FVector::ZeroVector;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_QuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_QuestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Quest Management
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void UpdateQuestProgress(const FString& QuestID, int32 ProgressAmount);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_QuestData GetQuestData(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_QuestData> GetActiveQuests();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_QuestData> GetCompletedQuests();

    // Quest Events
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnDinosaurKilled(const FString& DinosaurType);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnResourceGathered(const FString& ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnLocationReached(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnNPCInteraction(const FString& NPCName);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_QuestData> ActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_QuestData> CompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_QuestData> AvailableQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float QuestCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeSinceLastCheck;

    // Quest Templates
    void InitializeQuestTemplates();
    void CreateHuntQuests();
    void CreateGatherQuests();
    void CreateExplorationQuests();
    void CreateSurvivalQuests();

    // Quest Logic
    void CheckQuestCompletion();
    void ProcessQuestEvents();
    FQuest_QuestData* FindQuestByID(const FString& QuestID);
    bool IsQuestCompleted(const FQuest_QuestData& Quest);
};