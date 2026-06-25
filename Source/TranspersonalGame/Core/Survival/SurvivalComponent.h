#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

/**
 * USurvivalComponent
 * Tracks all survival stats for the player character:
 * Health, Hunger, Thirst, Stamina, Fear.
 *
 * Stats drain over time at configurable rates.
 * Hunger/Thirst depletion causes health damage.
 * Fear increases near predators and decreases in safe zones.
 *
 * Designed for ATranspersonalCharacter but usable on any AActor.
 */
UCLASS(ClassGroup = "Survival", meta = (BlueprintSpawnableComponent))
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

    // ── Stat accessors ────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival")
    float GetHealth() const { return Health; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival")
    float GetHunger() const { return Hunger; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival")
    float GetThirst() const { return Thirst; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival")
    float GetStamina() const { return Stamina; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival")
    float GetFear() const { return Fear; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival")
    bool IsAlive() const { return Health > 0.0f; }

    // ── Stat modifiers ────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ModifyHealth(float Delta);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ModifyHunger(float Delta);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ModifyThirst(float Delta);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ModifyStamina(float Delta);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ModifyFear(float Delta);

    // ── Biome-driven rate multipliers (set by BiomeManager each tick) ─────────
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetEnvironmentMultipliers(float ThirstMult, float StaminaMult, float FearMult);

    // ── Eat / Drink actions ───────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Eat(float NutritionValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Drink(float HydrationValue);

    // ── Drain rates (editable in editor) ─────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HungerDrainPerSecond;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float ThirstDrainPerSecond;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float FearDecayPerSecond;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StarvationDamagePerSecond;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float DehydrationDamagePerSecond;

    // ── Delegates ─────────────────────────────────────────────────────────────
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
    UPROPERTY(BlueprintAssignable, Category = "Survival")
    FOnDeath OnDeath;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float, NewHealth);
    UPROPERTY(BlueprintAssignable, Category = "Survival")
    FOnHealthChanged OnHealthChanged;

private:
    // ── Current stat values ───────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|Stats",
              meta = (AllowPrivateAccess = "true"))
    float Health;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|Stats",
              meta = (AllowPrivateAccess = "true"))
    float Hunger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|Stats",
              meta = (AllowPrivateAccess = "true"))
    float Thirst;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|Stats",
              meta = (AllowPrivateAccess = "true"))
    float Stamina;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|Stats",
              meta = (AllowPrivateAccess = "true"))
    float Fear;

    // ── Environment multipliers (set by BiomeManager) ─────────────────────────
    float EnvThirstMult;
    float EnvStaminaMult;
    float EnvFearMult;

    bool bDeathFired;

    static constexpr float MaxStat = 100.0f;
    static constexpr float MinStat = 0.0f;

    float ClampStat(float Value) const;
};
