#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "Quest_HuntingQuestManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_HuntingTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    FString TargetSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    float MinimumDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    bool bRequiresSpecificWeapon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting")
    FString RequiredWeaponType;

    FQuest_HuntingTarget()
    {
        TargetSpecies = TEXT("Unknown");
        RequiredCount = 1;
        CurrentCount = 0;
        MinimumDistance = 1000.0f;
        bRequiresSpecificWeapon = false;
        RequiredWeaponType = TEXT("Any");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_HuntingReward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
    TArray<FString> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
    float HealthBonus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
    float StaminaBonus;

    FQuest_HuntingReward()
    {
        ExperienceReward = 100;
        HealthBonus = 0.0f;
        StaminaBonus = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_HuntingQuestManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_HuntingQuestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Management")
    TArray<FQuest_HuntingTarget> ActiveHuntingTargets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Management")
    FQuest_HuntingReward CurrentReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Management")
    float QuestTimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Management")
    float ElapsedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Management")
    bool bIsQuestActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Management")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Management")
    FString QuestGiver;

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void StartHuntingQuest(const TArray<FQuest_HuntingTarget>& Targets, const FQuest_HuntingReward& Reward, float TimeLimit);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void RegisterKill(const FString& Species, const FString& WeaponUsed, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    bool CheckQuestCompletion();

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void CompleteQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    void FailQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    TArray<FString> GetQuestObjectives();

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    float GetQuestProgress();

    UFUNCTION(BlueprintCallable, Category = "Quest Management")
    FString GetQuestStatusText();

private:
    void UpdateQuestTimer(float DeltaTime);
    void CheckTimeLimit();
    bool ValidateKill(const FQuest_HuntingTarget& Target, const FString& WeaponUsed, float Distance);
};