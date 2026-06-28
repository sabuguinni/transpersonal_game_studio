// SurvivalComponent.h
// Core Systems Programmer #03 — Transpersonal Game Studio
// Survival stats component: health, hunger, thirst, stamina, fear, temperature
// Attached to TranspersonalCharacter in constructor

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

UENUM(BlueprintType)
enum class ECore_SurvivalStatus : uint8
{
    Healthy     UMETA(DisplayName = "Healthy"),
    Hungry      UMETA(DisplayName = "Hungry"),
    Thirsty     UMETA(DisplayName = "Thirsty"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Freezing    UMETA(DisplayName = "Freezing"),
    Overheating UMETA(DisplayName = "Overheating"),
    Critical    UMETA(DisplayName = "Critical"),
    Dead        UMETA(DisplayName = "Dead")
};

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

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Fear = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float MaxFear = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float BodyTemperature = 37.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float AmbientTemperature = 20.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSurvivalStatusChanged, ECore_SurvivalStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDied);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatChanged, FName, StatName, float, NewValue);

/**
 * USurvivalComponent
 * Manages all survival stats for the player character.
 * Ticks every second to drain hunger/thirst/stamina based on activity.
 * Emits delegates when stats cross critical thresholds.
 */
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

    // --- Survival Stats ---
    UPROPERTY(BlueprintReadWrite, Category = "Survival|Stats")
    FCore_SurvivalStats Stats;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Status")
    ECore_SurvivalStatus CurrentStatus;

    // --- Drain Rates (units per second) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HungerDrainRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float ThirstDrainRate = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaDrainRate = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaRegenRate = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float FearDecayRate = 2.0f;

    // --- Thresholds ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float CriticalHealthThreshold = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float CriticalHungerThreshold = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float CriticalThirstThreshold = 10.0f;

    // --- Delegates ---
    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnSurvivalStatusChanged OnSurvivalStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnPlayerDied OnPlayerDied;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnStatChanged OnStatChanged;

    // --- Blueprint-callable API ---

    /** Apply damage to health. Returns true if player died. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool ApplyDamage(float DamageAmount);

    /** Restore health by amount (clamped to MaxHealth). */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void HealHealth(float HealAmount);

    /** Consume food — restores hunger. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeFood(float NutritionValue);

    /** Drink water — restores thirst. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void DrinkWater(float HydrationValue);

    /** Use stamina (e.g. sprinting, attacking). */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool UseStamina(float StaminaCost);

    /** Increase fear (e.g. dinosaur nearby). */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddFear(float FearAmount);

    /** Reduce fear (e.g. safe shelter). */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ReduceFear(float FearAmount);

    /** Set ambient temperature (from world system). */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetAmbientTemperature(float Temperature);

    /** Check if player is sprinting (affects stamina drain). */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetSprinting(bool bIsSprinting);

    /** Get health as 0-1 normalized value. */
    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetHealthNormalized() const;

    /** Get hunger as 0-1 normalized value. */
    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetHungerNormalized() const;

    /** Get thirst as 0-1 normalized value. */
    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetThirstNormalized() const;

    /** Get stamina as 0-1 normalized value. */
    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetStaminaNormalized() const;

    /** Get fear as 0-1 normalized value. */
    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetFearNormalized() const;

    /** Is player alive? */
    UFUNCTION(BlueprintPure, Category = "Survival")
    bool IsAlive() const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    bool bIsSprinting = false;
    bool bIsDead = false;
    float TimeSinceLastDrain = 0.0f;
    static constexpr float DrainTickInterval = 1.0f;

    void TickSurvivalDrain(float DeltaTime);
    void UpdateSurvivalStatus();
    void ApplyHungerDamage();
    void ApplyThirstDamage();
    void ApplyTemperatureDamage(float DeltaTime);
    void BroadcastStatChange(FName StatName, float NewValue);
};
