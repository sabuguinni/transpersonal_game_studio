// SurvivalComponent.h
// Core Systems Programmer #03 — Cycle PROD_CYCLE_AUTO_20260625_005
// Prehistoric survival game — USurvivalComponent for hunger/thirst/stamina/fear/health

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

/** Survival stat change event — broadcast when any stat crosses a threshold */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSurvivalStatChanged, FName, StatName, float, NewValue);

/** Broadcast when the owner dies (health reaches 0) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOwnerDied);

/**
 * USurvivalComponent
 * Actor component that tracks survival stats for the player character and dinosaurs:
 * Health, Hunger, Thirst, Stamina, Fear, Temperature Stress.
 * Designed to tick every second (not every frame) for performance.
 * All stats are 0-100 floats. 0 = critical/dead for health; 100 = full.
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

    // ─── Stats ────────────────────────────────────────────────────────────────

    /** Current health (0-100). Reaches 0 → death. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats",
              meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Health = 100.f;

    /** Current hunger (0-100). Drains over time. 0 = starving → health drain. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats",
              meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Hunger = 100.f;

    /** Current thirst (0-100). Drains faster than hunger. 0 = dehydrated → health drain. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats",
              meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Thirst = 100.f;

    /** Current stamina (0-100). Drains during sprint/combat, recovers at rest. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats",
              meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Stamina = 100.f;

    /** Current fear level (0-100). 100 = panic → movement penalty + stamina drain. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats",
              meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Fear = 0.f;

    /** Temperature stress (0-100). Driven by BiomeManager::GetTemperatureAtLocation(). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats",
              meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float TemperatureStress = 0.f;

    // ─── Drain Rates (per second) ─────────────────────────────────────────────

    /** Hunger drain per second during normal activity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HungerDrainRate = 0.05f;

    /** Thirst drain per second during normal activity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float ThirstDrainRate = 0.08f;

    /** Stamina drain per second while sprinting */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaSprintDrainRate = 8.f;

    /** Stamina recovery per second while not sprinting */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaRecoveryRate = 5.f;

    /** Fear decay per second when no threat is visible */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float FearDecayRate = 2.f;

    /** Health drain per second when hunger OR thirst reaches 0 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StarvationHealthDrain = 0.5f;

    // ─── State Flags ──────────────────────────────────────────────────────────

    /** True while the owner is sprinting — set by TranspersonalCharacter */
    UPROPERTY(BlueprintReadWrite, Category = "Survival|State")
    bool bIsSprinting = false;

    /** True if owner is dead */
    UPROPERTY(BlueprintReadOnly, Category = "Survival|State")
    bool bIsDead = false;

    // ─── Events ───────────────────────────────────────────────────────────────

    /** Broadcast when any survival stat changes significantly (>5 points) */
    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnSurvivalStatChanged OnSurvivalStatChanged;

    /** Broadcast when health reaches 0 */
    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnOwnerDied OnOwnerDied;

    // ─── Public API ───────────────────────────────────────────────────────────

    /** Apply damage to health. Returns actual damage applied. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    float ApplyDamage(float DamageAmount);

    /** Consume food — restores hunger by Amount */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeFood(float Amount);

    /** Drink water — restores thirst by Amount */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void DrinkWater(float Amount);

    /** Increase fear (e.g., dinosaur spotted nearby) */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddFear(float Amount);

    /** Set temperature stress from BiomeManager (0-100) */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetTemperatureStress(float Stress);

    /** Returns true if the owner is in a critical survival state (any stat < 15) */
    UFUNCTION(BlueprintPure, Category = "Survival")
    bool IsInCriticalState() const;

    /** Returns a normalised danger score 0-1 combining all stats */
    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetOverallDangerScore() const;

private:
    /** Accumulator for tick-rate throttling (update every 1s not every frame) */
    float TickAccumulator = 0.f;

    /** Tick interval in seconds for stat drain calculations */
    static constexpr float StatTickInterval = 1.f;

    /** Internal stat update called every StatTickInterval seconds */
    void UpdateStats(float DeltaSeconds);

    /** Clamp a stat to [0, 100] and broadcast change event if delta > 5 */
    void SetStat(float& Stat, float NewValue, FName StatName);
};
