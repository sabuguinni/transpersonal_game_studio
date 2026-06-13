#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Eng_SurvivalSystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Thirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Temperature = 20.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SurvivalConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float HealthDecayRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float HungerDecayRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float ThirstDecayRate = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float StaminaRegenRate = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float FearDecayRate = 2.0f;
};

UCLASS()
class TRANSPERSONALGAME_API UEng_SurvivalSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UpdateSurvivalStats(AActor* Actor, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    FEng_SurvivalStats GetSurvivalStats(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetSurvivalStats(AActor* Actor, const FEng_SurvivalStats& NewStats);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsActorAlive(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyDamage(AActor* Actor, float Damage);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyHealing(AActor* Actor, float Healing);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeFood(AActor* Actor, float FoodValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeDrink(AActor* Actor, float DrinkValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddFear(AActor* Actor, float FearAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetTemperature(AActor* Actor, float NewTemperature);

private:
    UPROPERTY()
    TMap<AActor*, FEng_SurvivalStats> ActorSurvivalStats;

    UPROPERTY(EditAnywhere, Category = "Config")
    FEng_SurvivalConfig SurvivalConfig;

    void InitializeActorStats(AActor* Actor);
    void ProcessHealthEffects(AActor* Actor, FEng_SurvivalStats& Stats, float DeltaTime);
    void ProcessTemperatureEffects(AActor* Actor, FEng_SurvivalStats& Stats, float DeltaTime);
};