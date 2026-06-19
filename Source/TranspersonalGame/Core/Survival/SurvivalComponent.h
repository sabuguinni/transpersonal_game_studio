// Copyright Transpersonal Game Studio. All Rights Reserved.
// SurvivalComponent — Manages all survival stats for the prehistoric human player.
// Health, Hunger, Thirst, Stamina, Fear — all tick-driven with configurable drain rates.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSurvivalComponent, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSurvivalStatChanged, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDied);

/**
 * USurvivalComponent
 *
 * Attached to ATranspersonalCharacter. Drives all survival mechanics:
 * - Health:   Depleted by dinosaur attacks, falls, starvation, dehydration.
 * - Hunger:   Drains over time. At 0 → health drain begins.
 * - Thirst:   Drains faster than hunger. At 0 → health drain begins.
 * - Stamina:  Drains when running/climbing/fighting. Recovers when idle.
 * - Fear:     Increases near predators. High fear → reduced stamina regen, screen vignette.
 *
 * All values are 0.0–100.0 (percentage-based for easy Blueprint binding).
 */
UCLASS(ClassGroup=(Survival), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // ── Stat Values ──────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Survival, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Survival, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Survival, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Thirst;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Survival, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Stamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Survival, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Fear;

    // ── Drain Rates (units per second) ───────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SurvivalRates)
    float HungerDrainRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SurvivalRates)
    float ThirstDrainRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SurvivalRates)
    float StaminaRegenRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SurvivalRates)
    float FearDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SurvivalRates)
    float StarvationHealthDrain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SurvivalRates)
    float DehydrationHealthDrain;

    // ── Events ───────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = Survival)
    FOnSurvivalStatChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = Survival)
    FOnSurvivalStatChanged OnHungerChanged;

    UPROPERTY(BlueprintAssignable, Category = Survival)
    FOnSurvivalStatChanged OnThirstChanged;

    UPROPERTY(BlueprintAssignable, Category = Survival)
    FOnSurvivalStatChanged OnStaminaChanged;

    UPROPERTY(BlueprintAssignable, Category = Survival)
    FOnSurvivalStatChanged OnFearChanged;

    UPROPERTY(BlueprintAssignable, Category = Survival)
    FOnPlayerDied OnPlayerDied;

    // ── Mutators ─────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = Survival)
    void ConsumeStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = Survival)
    void IncreaseFear(float Amount);

    UFUNCTION(BlueprintCallable, Category = Survival)
    void RestoreHealth(float Amount);

    UFUNCTION(BlueprintCallable, Category = Survival)
    void RestoreStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = Survival)
    void RestoreHunger(float Amount);

    UFUNCTION(BlueprintCallable, Category = Survival)
    void RestoreThirst(float Amount);

    UFUNCTION(BlueprintCallable, Category = Survival)
    void ApplyDamage(float DamageAmount);

    // ── Getters (percentage 0.0–1.0) ─────────────────────────────────────────
    UFUNCTION(BlueprintPure, Category = Survival)
    float GetHealthPercentage() const { return Health / 100.0f; }

    UFUNCTION(BlueprintPure, Category = Survival)
    float GetHungerPercentage() const { return Hunger / 100.0f; }

    UFUNCTION(BlueprintPure, Category = Survival)
    float GetThirstPercentage() const { return Thirst / 100.0f; }

    UFUNCTION(BlueprintPure, Category = Survival)
    float GetStaminaPercentage() const { return Stamina / 100.0f; }

    UFUNCTION(BlueprintPure, Category = Survival)
    float GetFearPercentage() const { return Fear / 100.0f; }

    // ── Raw Getters ───────────────────────────────────────────────────────────
    UFUNCTION(BlueprintPure, Category = Survival)
    float GetHealth()  const { return Health; }

    UFUNCTION(BlueprintPure, Category = Survival)
    float GetHunger()  const { return Hunger; }

    UFUNCTION(BlueprintPure, Category = Survival)
    float GetThirst()  const { return Thirst; }

    UFUNCTION(BlueprintPure, Category = Survival)
    float GetStamina() const { return Stamina; }

    UFUNCTION(BlueprintPure, Category = Survival)
    float GetFear()    const { return Fear; }

    UFUNCTION(BlueprintPure, Category = Survival)
    bool IsAlive() const { return Health > 0.0f; }

    UFUNCTION(BlueprintPure, Category = Survival)
    bool IsCritical() const { return Health < 20.0f || Thirst < 10.0f || Hunger < 10.0f; }

private:
    bool bIsDead;

    void TickHunger(float DeltaTime);
    void TickThirst(float DeltaTime);
    void TickStamina(float DeltaTime);
    void TickFear(float DeltaTime);
    void CheckDeathCondition();
    void ClampStats();
};
