#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

/** Survival stat identifiers — used for UI binding and event dispatch */
UENUM(BlueprintType)
enum class ECore_SurvivalStat : uint8
{
    Health    UMETA(DisplayName = "Health"),
    Hunger    UMETA(DisplayName = "Hunger"),
    Thirst    UMETA(DisplayName = "Thirst"),
    Stamina   UMETA(DisplayName = "Stamina"),
    Fear      UMETA(DisplayName = "Fear"),
};

/** Snapshot of all survival stats — passed to UI and AI */
USTRUCT(BlueprintType)
struct FCore_SurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Health = 100.f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Hunger = 100.f;   // 100 = full, 0 = starving

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Thirst = 100.f;   // 100 = hydrated, 0 = dehydrated

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Stamina = 100.f;  // 100 = rested, 0 = exhausted

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Fear = 0.f;       // 0 = calm, 100 = terrified
};

/** Delegate fired when a stat crosses a critical threshold */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSurvivalStatCritical, ECore_SurvivalStat, Stat, float, CurrentValue);

/** Delegate fired on death */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDeath);

/**
 * USurvivalComponent
 *
 * Manages the five core survival stats for the primitive human player:
 *   Health, Hunger, Thirst, Stamina, Fear.
 *
 * Attach to ATranspersonalCharacter. Ticks at 1 Hz (TickInterval = 1.0f)
 * to drain stats over time. Stamina drains/recovers at 60 Hz via
 * ConsumeStamina() / RecoverStamina() called from the character movement.
 *
 * No spiritual content. All mechanics are physical survival.
 */
UCLASS(ClassGroup = (Survival), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

    // ── Lifecycle ────────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // ── Stat Accessors ───────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Survival")
    FCore_SurvivalStats GetCurrentStats() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetHealth()  const { return Stats.Health;  }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetHunger()  const { return Stats.Hunger;  }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetThirst()  const { return Stats.Thirst;  }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetStamina() const { return Stats.Stamina; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetFear()    const { return Stats.Fear;    }

    // ── Stat Modifiers ───────────────────────────────────────────────────────

    /** Apply damage (negative delta to Health). Triggers death if <= 0. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyDamage(float Amount);

    /** Eat food — restores Hunger by Amount (clamped to 100). */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Eat(float Amount);

    /** Drink water — restores Thirst by Amount (clamped to 100). */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Drink(float Amount);

    /** Consume stamina (called by movement component during sprint/jump). */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeStamina(float Amount);

    /** Recover stamina (called when not sprinting). */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void RecoverStamina(float Amount);

    /** Increase fear (dinosaur proximity, darkness, etc.). */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void IncreaseFear(float Amount);

    /** Decrease fear (safe shelter, fire, distance from predators). */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void DecreaseFear(float Amount);

    /** Returns true if the character is alive. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsAlive() const { return bIsAlive; }

    // ── Drain Rates (editable per biome/difficulty) ──────────────────────────

    /** Hunger drain per second (default: 0.5 units/s → ~200s to starve) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HungerDrainRate = 0.5f;

    /** Thirst drain per second (default: 0.8 units/s → ~125s to dehydrate) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float ThirstDrainRate = 0.8f;

    /** Stamina recovery per second when idle/walking */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaRecoveryRate = 10.f;

    /** Fear decay per second when no threats are nearby */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float FearDecayRate = 2.f;

    /** Health damage per second when Hunger <= 0 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StarvationDamageRate = 1.f;

    /** Health damage per second when Thirst <= 0 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float DehydrationDamageRate = 2.f;

    // ── Critical Thresholds ──────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float CriticalHealthThreshold = 20.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float CriticalHungerThreshold = 15.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float CriticalThirstThreshold = 15.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float CriticalStaminaThreshold = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float HighFearThreshold = 75.f;

    // ── Events ───────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnSurvivalStatCritical OnStatCritical;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnPlayerDeath OnPlayerDeath;

private:
    UPROPERTY(VisibleAnywhere, Category = "Survival|State",
              meta = (AllowPrivateAccess = "true"))
    FCore_SurvivalStats Stats;

    bool bIsAlive = true;

    // Track which stats have already fired their critical event this tick
    bool bHealthCriticalFired  = false;
    bool bHungerCriticalFired  = false;
    bool bThirstCriticalFired  = false;
    bool bStaminaCriticalFired = false;
    bool bFearHighFired        = false;

    /** Drain hunger/thirst over DeltaTime; apply starvation/dehydration damage */
    void TickNaturalDrains(float DeltaTime);

    /** Check thresholds and broadcast events */
    void CheckCriticalThresholds();

    /** Clamp all stats to [0, 100] */
    void ClampStats();
};
