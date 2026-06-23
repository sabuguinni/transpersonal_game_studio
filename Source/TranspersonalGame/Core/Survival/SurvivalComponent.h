// SurvivalComponent.h
// Core Systems Programmer #03 — Transpersonal Game Studio
// Survival stats component: health, hunger, thirst, stamina, temperature, fear
// Attaches to TranspersonalCharacter. Ticks every frame, drains stats over time.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

// Survival stat identifiers — used for BiomeManager modifier queries
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

// Cause of death — for analytics and respawn logic
UENUM(BlueprintType)
enum class ECore_DeathCause : uint8
{
    None        UMETA(DisplayName = "None"),
    Starvation  UMETA(DisplayName = "Starvation"),
    Dehydration UMETA(DisplayName = "Dehydration"),
    Hypothermia UMETA(DisplayName = "Hypothermia"),
    Hyperthermia UMETA(DisplayName = "Hyperthermia"),
    DinosaurAttack UMETA(DisplayName = "DinosaurAttack"),
    FallDamage  UMETA(DisplayName = "FallDamage"),
    Unknown     UMETA(DisplayName = "Unknown")
};

// Snapshot of all survival stats — used for UI and save system
USTRUCT(BlueprintType)
struct FCore_SurvivalSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Health = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Hunger = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Thirst = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Stamina = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Temperature = 37.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Fear = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsAlive = true;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    ECore_DeathCause DeathCause = ECore_DeathCause::None;
};

/**
 * USurvivalComponent
 *
 * Manages all survival stats for the player character.
 * Drains hunger/thirst over time, applies temperature effects,
 * handles health damage from critical stat failures.
 *
 * Designed to be modulated by BiomeManager — biomes apply
 * temperature offsets and drain rate multipliers.
 */
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── Stat Accessors ──────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetHealth() const { return Health; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetHunger() const { return Hunger; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetThirst() const { return Thirst; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetStamina() const { return Stamina; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetTemperature() const { return BodyTemperature; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetFear() const { return Fear; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsAlive() const { return bIsAlive; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    FCore_SurvivalSnapshot GetSnapshot() const;

    // ── Stat Modifiers ──────────────────────────────────────────────────────

    /** Apply damage to health (positive = damage, negative = heal) */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyHealthDelta(float Delta, ECore_DeathCause Cause = ECore_DeathCause::Unknown);

    /** Eat food — restores hunger by Amount */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Eat(float Amount);

    /** Drink water — restores thirst by Amount */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Drink(float Amount);

    /** Consume stamina (sprinting, climbing) */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeStamina(float Amount);

    /** Restore stamina (resting) */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void RestoreStamina(float Amount);

    /** Set ambient temperature from BiomeManager (°C) */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetAmbientTemperature(float AmbientCelsius);

    /** Add fear (dinosaur nearby, loud noise) */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddFear(float Amount);

    /** Reduce fear (safe zone, campfire) */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ReduceFear(float Amount);

    // ── Biome Modifiers (set by BiomeManager) ───────────────────────────────

    /** Multiplier on hunger drain rate (1.0 = normal, 1.5 = desert heat) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Biome")
    float HungerDrainMultiplier = 1.0f;

    /** Multiplier on thirst drain rate */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Biome")
    float ThirstDrainMultiplier = 1.0f;

    /** Multiplier on stamina drain rate */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Biome")
    float StaminaDrainMultiplier = 1.0f;

    // ── Configuration ────────────────────────────────────────────────────────

    /** Base hunger drain per second at rest */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float BaseHungerDrainPerSecond = 0.5f;

    /** Base thirst drain per second at rest */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float BaseThirstDrainPerSecond = 0.8f;

    /** Stamina drain per second while sprinting */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float SprintStaminaDrainPerSecond = 15.0f;

    /** Stamina restore per second while idle */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float StaminaRestorePerSecond = 8.0f;

    /** Health damage per second when hunger reaches 0 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float StarvationDamagePerSecond = 2.0f;

    /** Health damage per second when thirst reaches 0 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float DehydrationDamagePerSecond = 3.0f;

    /** Body temperature danger thresholds (°C) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float HypothermiaDangerTemp = 32.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float HyperthermiaDangerTemp = 42.0f;

    /** Fear decay rate per second (natural calm-down) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float FearDecayPerSecond = 2.0f;

    // ── Events ───────────────────────────────────────────────────────────────

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatCritical, ECore_SurvivalStat, Stat);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeath, AActor*, DeadActor, ECore_DeathCause, Cause);

    /** Fired when any stat drops below 20% */
    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnStatCritical OnStatCritical;

    /** Fired when health reaches 0 */
    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnDeath OnDeath;

private:
    // Current stat values
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|State", meta = (AllowPrivateAccess = "true"))
    float Health = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|State", meta = (AllowPrivateAccess = "true"))
    float Hunger = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|State", meta = (AllowPrivateAccess = "true"))
    float Thirst = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|State", meta = (AllowPrivateAccess = "true"))
    float Stamina = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|State", meta = (AllowPrivateAccess = "true"))
    float BodyTemperature = 37.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|State", meta = (AllowPrivateAccess = "true"))
    float Fear = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|State", meta = (AllowPrivateAccess = "true"))
    bool bIsAlive = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|State", meta = (AllowPrivateAccess = "true"))
    ECore_DeathCause LastDeathCause = ECore_DeathCause::None;

    float AmbientTemperature = 20.0f;
    bool bIsSprinting = false;

    // Internal helpers
    void TickHunger(float DeltaTime);
    void TickThirst(float DeltaTime);
    void TickStamina(float DeltaTime);
    void TickTemperature(float DeltaTime);
    void TickFear(float DeltaTime);
    void CheckCriticalStats();
    void TriggerDeath(ECore_DeathCause Cause);
    float ClampStat(float Value) const { return FMath::Clamp(Value, 0.0f, 100.0f); }
};
