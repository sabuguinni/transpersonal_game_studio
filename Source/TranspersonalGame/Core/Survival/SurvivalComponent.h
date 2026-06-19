// SurvivalComponent.h
// Core Systems — Agent #04 Performance Optimizer
// Cycle: PROD_CYCLE_AUTO_20260619_010
// Tick interval 0.25s — performance-optimized survival stats

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

// ECore_ prefix — RULE 2 compliant (unique across project)
UENUM(BlueprintType)
enum class ECore_SurvivalStat : uint8
{
    Health          UMETA(DisplayName = "Health"),
    Hunger          UMETA(DisplayName = "Hunger"),
    Thirst          UMETA(DisplayName = "Thirst"),
    Stamina         UMETA(DisplayName = "Stamina"),
    BodyTemperature UMETA(DisplayName = "Body Temperature"),
    Fear            UMETA(DisplayName = "Fear"),
};

// FCore_ prefix — RULE 2 compliant
USTRUCT(BlueprintType)
struct FCore_SurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Health = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float MaxHealth = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Hunger = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float MaxHunger = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Thirst = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float MaxThirst = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Stamina = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float MaxStamina = 100.0f;

    // Body temperature in Celsius — baseline 37.0
    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float BodyTemperature = 37.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Fear = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float MaxFear = 100.0f;
};

// Delegates — Blueprint-assignable for HUD binding
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatChanged, ECore_SurvivalStat, Stat, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDied);

UCLASS(ClassGroup = (Survival), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Actions ---
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Eat(float NutritionValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Drink(float HydrationValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Rest(float RecoveryAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetFear(float FearValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetAmbientTemperature(float Temperature);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetExerting(bool bExerting);

    // --- Queries ---
    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetStat(ECore_SurvivalStat Stat) const;

    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetStatNormalized(ECore_SurvivalStat Stat) const;

    UFUNCTION(BlueprintPure, Category = "Survival")
    bool IsDead() const;

    UFUNCTION(BlueprintPure, Category = "Survival")
    FCore_SurvivalStats GetAllStats() const { return Stats; }

    // --- Delegates ---
    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnStatChanged OnStatChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnPlayerDied OnPlayerDied;

    // --- Config (editable per character) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float HungerDrainPerSecond = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float ThirstDrainPerSecond = 0.08f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float HealthDrainFromHunger = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float HealthDrainFromThirst = 0.04f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float FearDecayPerSecond = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float StaminaRegenPerSecond = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float StaminaDrainPerSecond = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float AmbientTemperature = 22.0f;

private:
    UPROPERTY(VisibleAnywhere, Category = "Survival|State", meta = (AllowPrivateAccess = "true"))
    FCore_SurvivalStats Stats;

    UPROPERTY(VisibleAnywhere, Category = "Survival|State", meta = (AllowPrivateAccess = "true"))
    bool bIsExerting = false;

    UPROPERTY(VisibleAnywhere, Category = "Survival|State", meta = (AllowPrivateAccess = "true"))
    bool bIsDead = false;

    void CheckDeath();
};
