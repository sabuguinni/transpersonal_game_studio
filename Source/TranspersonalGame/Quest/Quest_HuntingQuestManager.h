#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Quest_HuntingQuestManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_HuntTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    FString TargetSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    int32 RequiredKills;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    int32 CurrentKills;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    float MinDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    bool bRequiresStealth;

    FQuest_HuntTarget()
    {
        TargetSpecies = TEXT("Raptor");
        RequiredKills = 1;
        CurrentKills = 0;
        MinDistance = 500.0f;
        bRequiresStealth = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_HuntingReward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    FString ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    float ExperiencePoints;

    FQuest_HuntingReward()
    {
        ItemName = TEXT("Meat");
        Quantity = 5;
        ExperiencePoints = 100.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_HuntingQuestManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_HuntingQuestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting Quests")
    TArray<FQuest_HuntTarget> ActiveHuntTargets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting Quests")
    TArray<FQuest_HuntingReward> QuestRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting Quests")
    float TrackingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting Quests")
    bool bShowTrackingMarkers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting Quests")
    int32 MaxActiveHunts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting Quests")
    float HuntTimeLimit;

    UFUNCTION(BlueprintCallable, Category = "Hunting Quests")
    void StartHuntingQuest(const FString& TargetSpecies, int32 RequiredKills, bool bStealth);

    UFUNCTION(BlueprintCallable, Category = "Hunting Quests")
    void RegisterKill(const FString& KilledSpecies, float Distance, bool bWasStealth);

    UFUNCTION(BlueprintCallable, Category = "Hunting Quests")
    bool CheckQuestCompletion(const FString& TargetSpecies);

    UFUNCTION(BlueprintCallable, Category = "Hunting Quests")
    void CompleteHuntingQuest(const FString& TargetSpecies);

    UFUNCTION(BlueprintCallable, Category = "Hunting Quests")
    TArray<FString> GetActiveHuntTargets();

    UFUNCTION(BlueprintCallable, Category = "Hunting Quests")
    float GetHuntProgress(const FString& TargetSpecies);

    UFUNCTION(BlueprintCallable, Category = "Hunting Quests")
    void UpdateTrackingMarkers();

    UFUNCTION(BlueprintCallable, Category = "Hunting Quests")
    void CancelHuntingQuest(const FString& TargetSpecies);

private:
    void InitializeDefaultHunts();
    void CheckHuntTimeouts();
    FQuest_HuntTarget* FindHuntTarget(const FString& TargetSpecies);
};