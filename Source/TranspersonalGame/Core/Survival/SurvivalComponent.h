#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

/**
 * ESurvivalStatType — identifies each tracked survival stat.
 * Prefixed Core_ to avoid ODR collision across the 19-agent codebase.
 */
UENUM(BlueprintType)
enum class ECore_SurvivalStat : uint8
{
    Health      UMETA(DisplayName = "Health"),
    Hunger      UMETA(DisplayName = "Hunger"),
    Thirst      UMETA(DisplayName = "Thirst"),
    Stamina     UMETA(DisplayName = "Stamina"),
    Temperature UMETA(DisplayName = "Temperature"),
    Fear        UMETA(DisplayName = "Fear")
};

/**
 * FCore_SurvivalStatData — runtime data for a single survival stat.
 * Current, min, max, drain rate per second, critical threshold.
 */
USTRUCT(BlueprintType)
struct FCore_SurvivalStatData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Current = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Min = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Max = 100.0f;

    /** Units drained per second during normal activity. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float DrainRatePerSecond = 0.5f;

    /** Below this value the stat is considered critical (triggers UI warning). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float CriticalThreshold = 20.0f;

    bool IsCritical() const { return Current <= CriticalThreshold; }
    bool IsEmpty()    const { return Current <= Min; }
    float NormalizedValue() const { return (Max > Min) ? (Current - Min) / (Max - Min) : 0.0f; }
};

/**
 * USurvivalComponent — manages all survival stats for a prehistoric human character.
 *
 * Attach to ATranspersonalCharacter (or any APawn) to enable hunger, thirst,
 * stamina, temperature, and fear mechanics. Stats drain passively each tick;
 * external systems (crafting, combat, biome) call Modify* methods to adjust them.
 *
 * Design: Casey Muratori-style — testable in isolation, no hidden global state.
 */
UCLASS(ClassGroup = "TranspersonalGame", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

    // ── Lifecycle ────────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // ── Stat Access ──────────────────────────────────────────────────────────

    /** Returns normalised value [0,1] for the given stat. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetStatNormalized(ECore_SurvivalStat Stat) const;

    /** Returns raw current value for the given stat. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetStatCurrent(ECore_SurvivalStat Stat) const;

    /** Returns true if the stat is below its critical threshold. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsStatCritical(ECore_SurvivalStat Stat) const;

    // ── Stat Modification ────────────────────────────────────────────────────

    /**
     * Add (positive) or remove (negative) from a stat.
     * @param Delta  Amount to add; clamped to [Min, Max].
     */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ModifyStat(ECore_SurvivalStat Stat, float Delta);

    /** Convenience: apply damage directly to Health. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyDamage(float Amount);

    /** Convenience: eat food — restores Hunger, slight Thirst drain. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeFood(float NutritionValue);

    /** Convenience: drink water — restores Thirst. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void DrinkWater(float HydrationValue);

    /** Set ambient temperature modifier (from biome system). Range -1..+1. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetAmbientTemperatureModifier(float Modifier);

    /** Increase fear (predator nearby, loud noise, etc.). */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddFear(float Amount);

    /** Reduce fear (safe area, fire nearby, etc.). */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ReduceFear(float Amount);

    // ── State Queries ────────────────────────────────────────────────────────

    /** True when Health <= 0. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsDead() const;

    /** True when Stamina is critically low (can't sprint). */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsExhausted() const;

    // ── Configurable Drain Rates (editable per-character) ────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HungerDrainRate = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float ThirstDrainRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaRecoveryRate = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float FearDecayRate = 2.0f;

    /** Health drain per second when Hunger is empty. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StarvationDamageRate = 1.0f;

    /** Health drain per second when Thirst is empty. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float DehydrationDamageRate = 2.0f;

private:
    // ── Internal stat storage ─────────────────────────────────────────────────
    FCore_SurvivalStatData HealthStat;
    FCore_SurvivalStatData HungerStat;
    FCore_SurvivalStatData ThirstStat;
    FCore_SurvivalStatData StaminaStat;
    FCore_SurvivalStatData TemperatureStat;
    FCore_SurvivalStatData FearStat;

    float AmbientTemperatureModifier = 0.0f;
    bool bIsSprinting = false;

    /** Returns a mutable reference to the stat data for the given enum. */
    FCore_SurvivalStatData& GetStatData(ECore_SurvivalStat Stat);
    const FCore_SurvivalStatData& GetStatDataConst(ECore_SurvivalStat Stat) const;

    void TickPassiveDrain(float DeltaTime);
    void TickStaminaRecovery(float DeltaTime);
    void TickFearDecay(float DeltaTime);
    void TickStarvationAndDehydration(float DeltaTime);
    void TickTemperatureEffect(float DeltaTime);
};
