#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Quest_ResourceQuestManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ResourceTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Target")
    FString ResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Target")
    int32 RequiredAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Target")
    int32 CurrentAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Target")
    FString SourceLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Target")
    float QualityThreshold;

    FQuest_ResourceTarget()
    {
        ResourceType = TEXT("Stone");
        RequiredAmount = 10;
        CurrentAmount = 0;
        SourceLocation = TEXT("Rocky Hills");
        QualityThreshold = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ResourceReward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    FString RewardType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    int32 Amount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    float CraftingExperience;

    FQuest_ResourceReward()
    {
        RewardType = TEXT("Tool Blueprint");
        Amount = 1;
        CraftingExperience = 75.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_ResourceQuestManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_ResourceQuestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Quests")
    TArray<FQuest_ResourceTarget> ActiveResourceTargets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Quests")
    TArray<FQuest_ResourceReward> QuestRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Quests")
    float SearchRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Quests")
    bool bShowResourceMarkers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Quests")
    int32 MaxActiveGathering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Quests")
    float GatheringTimeLimit;

    UFUNCTION(BlueprintCallable, Category = "Resource Quests")
    void StartResourceQuest(const FString& ResourceType, int32 RequiredAmount, const FString& Location);

    UFUNCTION(BlueprintCallable, Category = "Resource Quests")
    void RegisterResourceGathered(const FString& ResourceType, int32 Amount, float Quality);

    UFUNCTION(BlueprintCallable, Category = "Resource Quests")
    bool CheckResourceQuestCompletion(const FString& ResourceType);

    UFUNCTION(BlueprintCallable, Category = "Resource Quests")
    void CompleteResourceQuest(const FString& ResourceType);

    UFUNCTION(BlueprintCallable, Category = "Resource Quests")
    TArray<FString> GetActiveResourceTargets();

    UFUNCTION(BlueprintCallable, Category = "Resource Quests")
    float GetResourceProgress(const FString& ResourceType);

    UFUNCTION(BlueprintCallable, Category = "Resource Quests")
    void UpdateResourceMarkers();

    UFUNCTION(BlueprintCallable, Category = "Resource Quests")
    void CancelResourceQuest(const FString& ResourceType);

    UFUNCTION(BlueprintCallable, Category = "Resource Quests")
    TArray<FString> GetNearbyResources(float Range);

private:
    void InitializeDefaultResourceQuests();
    void CheckGatheringTimeouts();
    FQuest_ResourceTarget* FindResourceTarget(const FString& ResourceType);
    bool ValidateResourceQuality(float Quality, float Threshold);
};