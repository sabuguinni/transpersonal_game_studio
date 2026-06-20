// SurvivalComponent.h
// Core Systems Programmer #03 — PROD_CYCLE_AUTO_20260620_001
// Survival stats component: health, hunger, thirst, stamina, temperature, fear
// Attaches to TranspersonalCharacter. No spiritual content — pure survival mechanics.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

UENUM(BlueprintType)
enum class ECore_SurvivalStat : uint8
{
    Health      UMETA(DisplayName = "Health"),
    Hunger      UMETA(DisplayName = "Hunger"),
    Thirst      UMETA(DisplayName = "Thirst"),
    Stamina     UMETA(DisplayName = "Stamina"),
    Temperature UMETA(DisplayName = "Temperature"),
    Fear        UMETA(DisplayName = "Fear"),
};

USTRUCT(BlueprintType)
struct FCore_SurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Hunger = 100.0f;   // 100 = full, 0 = starving

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxHunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Thirst = 100.0f;   // 100 = hydrated, 0 = dehydrated

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxThirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float BodyTemperature = 37.0f;  // Celsius — normal human temp

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Fear = 0.0f;  // 0 = calm, 100 = panic

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxFear = 100.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatChanged, ECore_SurvivalStat, Stat, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDied);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCriticalStat, ECore_SurvivalStat, Stat);

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), DisplayName = "Survival Component")
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

    // --- Delegates ---
    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnStatChanged OnStatChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnPlayerDied OnPlayerDied;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnCriticalStat OnCriticalStat;

    // --- Config ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float HungerDrainRate = 1.0f;   // units/second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float ThirstDrainRate = 1.5f;   // units/second — thirst drains faster

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float StaminaRegenRate = 10.0f; // units/second when resting

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float StaminaDrainRate = 20.0f; // units/second when sprinting

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float FearDecayRate = 5.0f;     // units/second — fear fades over time

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float CriticalThreshold = 20.0f; // below this = critical state

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float StarvationDamageRate = 2.0f; // health/second when starving

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float DehydrationDamageRate = 3.0f; // health/second when dehydrated

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float HypothermiaDamageRate = 1.5f; // health/second when too cold

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float HyperthermiaThreshold = 40.0f; // body temp above this = heatstroke

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float HypothermiaThreshold = 35.0f;  // body temp below this = hypothermia

    // --- State ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|State")
    FCore_SurvivalStats Stats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|State")
    bool bIsSprinting = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|State")
    bool bIsDead = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|State")
    float AmbientTemperature = 20.0f; // Set by BiomeManager each tick

    // --- Public API ---
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyDamage(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Eat(float NutritionValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Drink(float HydrationValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetSprinting(bool bSprinting);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddFear(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetAmbientTemperature(float Celsius);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Heal(float Amount);

    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetHungerPercent() const;

    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetThirstPercent() const;

    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetStaminaPercent() const;

    UFUNCTION(BlueprintPure, Category = "Survival")
    bool IsStarving() const;

    UFUNCTION(BlueprintPure, Category = "Survival")
    bool IsDehydrated() const;

    UFUNCTION(BlueprintPure, Category = "Survival")
    bool IsHypothermic() const;

    UFUNCTION(BlueprintPure, Category = "Survival")
    bool IsHyperthermic() const;

    UFUNCTION(BlueprintPure, Category = "Survival")
    bool IsInCriticalState() const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void TickHunger(float DeltaTime);
    void TickThirst(float DeltaTime);
    void TickStamina(float DeltaTime);
    void TickTemperature(float DeltaTime);
    void TickFear(float DeltaTime);
    void CheckCriticalStates();
    void BroadcastStatChange(ECore_SurvivalStat Stat, float NewValue);
    void Die();

    bool bHungerCriticalFired = false;
    bool bThirstCriticalFired = false;
    bool bHealthCriticalFired = false;
};
