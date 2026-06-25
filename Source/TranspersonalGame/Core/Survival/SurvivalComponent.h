#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

UENUM(BlueprintType)
enum class ECore_SurvivalStatus : uint8
{
    Healthy     UMETA(DisplayName = "Healthy"),
    Hungry      UMETA(DisplayName = "Hungry"),
    Starving    UMETA(DisplayName = "Starving"),
    Thirsty     UMETA(DisplayName = "Thirsty"),
    Dehydrated  UMETA(DisplayName = "Dehydrated"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Panicking   UMETA(DisplayName = "Panicking"),
    Critical    UMETA(DisplayName = "Critical")
};

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
    float Temperature = 37.f;
};

/**
 * USurvivalComponent
 * Manages prehistoric survival stats: hunger, thirst, stamina, fear, temperature.
 * Ticks every second to drain stats. Triggers damage when stats hit critical thresholds.
 * Attach to ATranspersonalCharacter.
 */
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── Stat accessors ──────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetHealth() const { return Stats.Health; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetHunger() const { return Stats.Hunger; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetThirst() const { return Stats.Thirst; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetStamina() const { return Stats.Stamina; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetFear() const { return Stats.Fear; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetTemperature() const { return Stats.Temperature; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    ECore_SurvivalStatus GetCurrentStatus() const { return CurrentStatus; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsAlive() const { return Stats.Health > 0.f; }

    // ── Stat modifiers ──────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeFood(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void DrinkWater(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void RestoreStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyFear(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ReduceFear(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyDamage(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void HealHealth(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UseStamina(float Amount);

    // ── Drain rates (editable per species / difficulty) ─────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HungerDrainPerSecond = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float ThirstDrainPerSecond = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaRegenPerSecond = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float FearDecayPerSecond = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StarvationDamagePerSecond = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float DehydrationDamagePerSecond = 1.5f;

    // ── Thresholds ──────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float HungerWarningThreshold = 30.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float ThirstWarningThreshold = 25.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float StaminaExhaustionThreshold = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float FearPanicThreshold = 80.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float CriticalHealthThreshold = 20.f;

    // ── Blueprint events ────────────────────────────────────────────────────

    UFUNCTION(BlueprintImplementableEvent, Category = "Survival|Events")
    void OnHungerWarning(float CurrentHunger);

    UFUNCTION(BlueprintImplementableEvent, Category = "Survival|Events")
    void OnThirstWarning(float CurrentThirst);

    UFUNCTION(BlueprintImplementableEvent, Category = "Survival|Events")
    void OnStaminaExhausted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Survival|Events")
    void OnFearPanic(float CurrentFear);

    UFUNCTION(BlueprintImplementableEvent, Category = "Survival|Events")
    void OnDeath();

    UFUNCTION(BlueprintImplementableEvent, Category = "Survival|Events")
    void OnCriticalHealth(float CurrentHealth);

    // ── Debug ────────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Survival|Debug")
    void PrintSurvivalStats() const;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    FCore_SurvivalStats Stats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    ECore_SurvivalStatus CurrentStatus;

    bool bIsDead = false;
    bool bIsRunning = false;

    float AccumulatedTime = 0.f;
    static constexpr float TickInterval = 1.0f; // drain every 1 second

    void DrainStats(float DeltaSeconds);
    void ApplyStarvationDamage();
    void ApplyDehydrationDamage();
    void UpdateStatus();
    void ClampStats();
};
