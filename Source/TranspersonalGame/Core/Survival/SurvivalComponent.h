#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

/**
 * Survival stat thresholds and danger levels for the prehistoric survival game.
 * Tracks hunger, thirst, stamina, health, temperature, and fear.
 */
UENUM(BlueprintType)
enum class ECore_SurvivalStatLevel : uint8
{
    Critical    UMETA(DisplayName = "Critical"),
    Low         UMETA(DisplayName = "Low"),
    Normal      UMETA(DisplayName = "Normal"),
    High        UMETA(DisplayName = "High"),
    Full        UMETA(DisplayName = "Full")
};

/**
 * Snapshot of all survival stats for replication and UI.
 */
USTRUCT(BlueprintType)
struct FCore_SurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Health = 100.f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Hunger = 100.f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Thirst = 100.f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Stamina = 100.f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Temperature = 37.f;   // Celsius — normal body temp

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Fear = 0.f;           // 0=calm, 100=panic
};

/**
 * USurvivalComponent
 * Manages all survival statistics for the player character.
 * Attached to ATranspersonalCharacter in constructor.
 *
 * Drain rates (per second, at rest):
 *   Hunger  : -0.5 / s
 *   Thirst  : -0.8 / s
 *   Stamina : regenerates +5 / s when not sprinting; drains -10 / s when sprinting
 *   Health  : drains when Hunger or Thirst reach Critical
 *   Fear    : spikes near predators, decays naturally
 */
UCLASS(ClassGroup = (Survival), meta = (BlueprintSpawnableComponent), DisplayName = "Survival Component")
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

    // ── Tick ──────────────────────────────────────────────────────────────────
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // ── Accessors ─────────────────────────────────────────────────────────────

    /** Returns a snapshot of all current survival stats. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    FCore_SurvivalStats GetSurvivalStats() const;

    /** Returns the danger level for a given stat value (0-100 scale). */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    static ECore_SurvivalStatLevel GetStatLevel(float Value);

    // ── Modifiers ─────────────────────────────────────────────────────────────

    /** Consume food — restores Hunger. Amount 0-100. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeFood(float Amount);

    /** Drink water — restores Thirst. Amount 0-100. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void DrinkWater(float Amount);

    /** Apply damage to Health (positive = damage, negative = heal). */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyHealthDelta(float Delta);

    /** Spike Fear by Amount. Decays naturally over time. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddFear(float Amount);

    /** Called by movement component when sprinting starts/stops. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetSprinting(bool bIsSprinting);

    /** Returns true if the character is alive (Health > 0). */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsAlive() const;

    // ── Drain rates (designer-tunable) ────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HungerDrainPerSecond = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float ThirstDrainPerSecond = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaRegenPerSecond = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaDrainSprintPerSecond = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float FearDecayPerSecond = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HealthDrainStarvationPerSecond = 1.0f;

    // ── Current stats (read-only from Blueprint) ──────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats",
              meta = (AllowPrivateAccess = "true"))
    float Health = 100.f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats",
              meta = (AllowPrivateAccess = "true"))
    float Hunger = 100.f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats",
              meta = (AllowPrivateAccess = "true"))
    float Thirst = 100.f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats",
              meta = (AllowPrivateAccess = "true"))
    float Stamina = 100.f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats",
              meta = (AllowPrivateAccess = "true"))
    float Temperature = 37.f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats",
              meta = (AllowPrivateAccess = "true"))
    float Fear = 0.f;

protected:
    virtual void BeginPlay() override;

private:
    bool bSprinting = false;

    /** Clamp a stat to [0, 100]. */
    static float ClampStat(float Value) { return FMath::Clamp(Value, 0.f, 100.f); }

    /** Tick hunger and thirst drain. */
    void TickHungerThirst(float DeltaTime);

    /** Tick stamina regen/drain based on sprint state. */
    void TickStamina(float DeltaTime);

    /** Tick health drain when starving or dehydrated. */
    void TickHealthDrain(float DeltaTime);

    /** Tick fear decay. */
    void TickFear(float DeltaTime);
};
