#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

/** Tick interval categories for survival drain rates */
UENUM(BlueprintType)
enum class ECore_SurvivalState : uint8
{
    Healthy     UMETA(DisplayName = "Healthy"),
    Hungry      UMETA(DisplayName = "Hungry"),
    Thirsty     UMETA(DisplayName = "Thirsty"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Critical    UMETA(DisplayName = "Critical"),
    Dead        UMETA(DisplayName = "Dead")
};

/** Snapshot of all survival stats — used for UI and save/load */
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
    float Fear = 0.f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float BodyTemperature = 37.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    ECore_SurvivalState State = ECore_SurvivalState::Healthy;
};

/**
 * USurvivalComponent
 *
 * Manages all survival statistics for a prehistoric human character:
 * health, hunger, thirst, stamina, fear, and body temperature.
 * Drains occur on a configurable tick interval. Critical thresholds
 * trigger health damage. Death is broadcast via OnDeath delegate.
 *
 * Attach to ATranspersonalCharacter in its constructor.
 */
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), DisplayName = "Survival Component")
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

    // ── UActorComponent overrides ──────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // ── Stat accessors ─────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival")
    FCore_SurvivalStats GetStats() const { return Stats; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    ECore_SurvivalState GetSurvivalState() const { return Stats.State; }

    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetHealth() const { return Stats.Health; }

    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetHunger() const { return Stats.Hunger; }

    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetThirst() const { return Stats.Thirst; }

    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetStamina() const { return Stats.Stamina; }

    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetFear() const { return Stats.Fear; }

    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetBodyTemperature() const { return Stats.BodyTemperature; }

    UFUNCTION(BlueprintPure, Category = "Survival")
    bool IsAlive() const { return Stats.State != ECore_SurvivalState::Dead; }

    // ── Stat modifiers ─────────────────────────────────────────────────────
    /** Apply direct damage (positive = damage, negative = heal) */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyDamage(float Amount);

    /** Consume food — restores hunger. Amount 0-100. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeFood(float Amount);

    /** Drink water — restores thirst. Amount 0-100. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void DrinkWater(float Amount);

    /** Add or remove stamina. Negative = spend, positive = recover. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ModifyStamina(float Delta);

    /** Increase fear (e.g. dinosaur nearby). Clamped 0-100. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddFear(float Amount);

    /** Reduce fear (e.g. safe shelter). Clamped 0-100. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ReduceFear(float Amount);

    /** Set ambient temperature — affects body temperature drift. */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetAmbientTemperature(float Celsius);

    // ── Delegates ──────────────────────────────────────────────────────────
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnDeath OnDeath;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateChanged, ECore_SurvivalState, NewState);
    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnStateChanged OnStateChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float, NewHealth);
    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnHealthChanged OnHealthChanged;

    // ── Configurable drain rates (per second) ──────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float HungerDrainRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float ThirstDrainRate = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float StaminaRecoveryRate = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float FearDecayRate = 1.0f;

    /** Health damage per second when hunger or thirst reaches 0 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float StarvationDamageRate = 2.0f;

    /** Critical threshold — below this triggers Critical state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float CriticalHealthThreshold = 20.0f;

    /** Ambient temperature in Celsius (set by world/biome systems) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|Environment")
    float AmbientTemperature = 28.0f;

private:
    UPROPERTY(VisibleAnywhere, Category = "Survival|State",
              meta = (AllowPrivateAccess = "true"))
    FCore_SurvivalStats Stats;

    float AccumulatedTime = 0.f;
    static constexpr float TickInterval = 1.0f; // drain tick every 1 second

    void DrainStats(float DeltaSeconds);
    void UpdateBodyTemperature(float DeltaSeconds);
    void ApplyStarvationDamage(float DeltaSeconds);
    void RecalculateState();
    void SetStat(float& Stat, float NewValue);
};
