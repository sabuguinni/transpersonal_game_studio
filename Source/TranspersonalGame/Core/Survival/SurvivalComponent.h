#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "SurvivalComponent.generated.h"

// Forward declarations
class ABiomeManager;

/**
 * Survival stat identifiers — used for generic stat queries and UI binding.
 */
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

/**
 * Snapshot of all survival stats — used for UI, save/load, and AI queries.
 */
USTRUCT(BlueprintType)
struct FCore_SurvivalSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Health = 100.f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Hunger = 100.f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Thirst = 100.f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Stamina = 100.f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Temperature = 37.f;   // Celsius — normal body temp

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float Fear = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsAlive = true;
};

/**
 * USurvivalComponent
 *
 * Manages all survival statistics for the owning actor (player character or NPC).
 * Tracks health, hunger, thirst, stamina, temperature, and fear.
 * Applies biome-driven modifiers when a BiomeManager is present in the world.
 *
 * Designed for ATranspersonalCharacter but works on any AActor.
 */
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

    // ─── Tick ───────────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // ─── Stat Accessors ─────────────────────────────────────────────────────

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

    /** Returns a full snapshot of all stats — useful for UI and save/load. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    FCore_SurvivalSnapshot GetSnapshot() const;

    /** Generic stat query by enum — returns normalised 0-1 value. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetStatNormalised(ECore_SurvivalStat Stat) const;

    // ─── Stat Modifiers ─────────────────────────────────────────────────────

    /** Apply damage to health. Returns actual damage applied. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    float ApplyDamage(float Amount);

    /** Restore health. Returns actual amount healed. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    float Heal(float Amount);

    /** Consume food — increases Hunger. Returns actual amount consumed. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    float Eat(float Amount);

    /** Consume water — increases Thirst. Returns actual amount consumed. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    float Drink(float Amount);

    /** Spend stamina (running, climbing, fighting). Returns actual cost. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    float SpendStamina(float Amount);

    /** Add fear (predator proximity, combat, darkness). Clamped 0-100. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddFear(float Amount);

    /** Reduce fear (safety, rest, fire). Clamped 0-100. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ReduceFear(float Amount);

    // ─── Configuration ───────────────────────────────────────────────────────

    /** Rate at which hunger decreases per second (default: 0.5 units/s). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HungerDecayRate = 0.5f;

    /** Rate at which thirst decreases per second (default: 0.8 units/s). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float ThirstDecayRate = 0.8f;

    /** Rate at which stamina regenerates per second when not sprinting (default: 10 units/s). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaRegenRate = 10.f;

    /** Rate at which fear decays per second when no threat is present (default: 2 units/s). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float FearDecayRate = 2.f;

    /** Health damage per second when hunger reaches 0 (default: 1 unit/s). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Damage")
    float StarvationDamageRate = 1.f;

    /** Health damage per second when thirst reaches 0 (default: 2 units/s). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Damage")
    float DehydrationDamageRate = 2.f;

    /** Ambient temperature modifier from biome (set by BiomeManager integration). */
    UPROPERTY(BlueprintReadOnly, Category = "Survival|Biome")
    float BiomeTemperatureModifier = 0.f;

    /** Humidity modifier from biome — affects thirst decay rate. */
    UPROPERTY(BlueprintReadOnly, Category = "Survival|Biome")
    float BiomeHumidityModifier = 0.f;

    // ─── Events ──────────────────────────────────────────────────────────────

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnDeath OnDeath;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatChanged, ECore_SurvivalStat, Stat);
    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnStatChanged OnStatChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLowStat, ECore_SurvivalStat, Stat);
    /** Fires once when a stat drops below its critical threshold. */
    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnLowStat OnLowStat;

private:
    // ─── Internal Stats ──────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, Category = "Survival|Debug", meta = (AllowPrivateAccess = "true"))
    float Health = 100.f;

    UPROPERTY(VisibleAnywhere, Category = "Survival|Debug", meta = (AllowPrivateAccess = "true"))
    float Hunger = 100.f;

    UPROPERTY(VisibleAnywhere, Category = "Survival|Debug", meta = (AllowPrivateAccess = "true"))
    float Thirst = 100.f;

    UPROPERTY(VisibleAnywhere, Category = "Survival|Debug", meta = (AllowPrivateAccess = "true"))
    float Stamina = 100.f;

    UPROPERTY(VisibleAnywhere, Category = "Survival|Debug", meta = (AllowPrivateAccess = "true"))
    float BodyTemperature = 37.f;

    UPROPERTY(VisibleAnywhere, Category = "Survival|Debug", meta = (AllowPrivateAccess = "true"))
    float Fear = 0.f;

    UPROPERTY(VisibleAnywhere, Category = "Survival|Debug", meta = (AllowPrivateAccess = "true"))
    bool bIsAlive = true;

    // ─── Critical threshold tracking ─────────────────────────────────────────
    bool bHungerCriticalFired = false;
    bool bThirstCriticalFired = false;
    bool bHealthCriticalFired = false;
    bool bStaminaCriticalFired = false;

    // ─── Internal helpers ────────────────────────────────────────────────────
    void TickHunger(float DeltaTime);
    void TickThirst(float DeltaTime);
    void TickStamina(float DeltaTime);
    void TickFear(float DeltaTime);
    void TickTemperature(float DeltaTime);
    void CheckCriticalThresholds();
    void TriggerDeath();

    /** Clamp value to [0, Max] and return clamped result. */
    static float ClampStat(float Value, float Max = 100.f);
};
