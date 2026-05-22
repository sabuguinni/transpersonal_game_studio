#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "Quest_ResourceGathering.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ResourceItem
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Resource")
    FString ResourceName;

    UPROPERTY(BlueprintReadWrite, Category = "Resource")
    int32 RequiredAmount;

    UPROPERTY(BlueprintReadWrite, Category = "Resource")
    int32 CurrentAmount;

    UPROPERTY(BlueprintReadWrite, Category = "Resource")
    FVector SpawnLocation;

    FQuest_ResourceItem()
    {
        ResourceName = TEXT("Unknown");
        RequiredAmount = 1;
        CurrentAmount = 0;
        SpawnLocation = FVector::ZeroVector;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ResourceGathering : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ResourceGathering();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_ResourceItem> RequiredResources;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    bool bIsQuestActive;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    bool bIsQuestCompleted;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    float QuestTimeLimit;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    float QuestStartTime;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void StartQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void AddResourceToInventory(const FString& ResourceName, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CheckQuestCompletion();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void SpawnResourceNodes();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    float GetRemainingTime() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FString GetQuestStatus() const;

private:
    void InitializeResourceRequirements();
    void UpdateQuestProgress();
    void CompleteQuest();
    void FailQuest();
};