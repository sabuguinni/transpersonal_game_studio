// SurvivalComponent.h
// Core Systems Programmer #03 — PROD_CYCLE_AUTO_20260630_009
// Survival stats component: Health, Hunger, Thirst, Stamina, Temperature, Fear
// Attaches to TranspersonalCharacter. Ticks every second.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

// ─── Enums (global scope — UHT requirement) ─────────────────────────────────

UENUM(BlueprintType)
enum class ECore_SurvivalStatus : uint8
{
    Healthy     UMETA(DisplayName = "Healthy"),
    Hungry      UMETA(DisplayName = "Hungry"),
    Starving    UMETA(DisplayName = "Starving"),
    Thirsty     UMETA(DisplayName = "Thirsty"),
    Dehydrated  UMETA(DisplayName = "Dehydrated"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Hypothermic UMETA(DisplayName = "Hypothermic"),
    Hyperthermic UMETA(DisplayName = "Hyperthermic"),
    Critical    UMETA(DisplayName = "Critical"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECore_TemperatureZone : uint8
{
    Freezing    UMETA(DisplayName = "Freezing"),    // < -10°C
    Cold        UMETA(DisplayName = "Cold"),         // -10 to 10°C
    Comfortable UMETA(DisplayName = "Comfortable"),  // 10 to 30°C
    Hot         UMETA(DisplayName = "Hot"),          // 30 to 45°C
    Lethal      UMETA(DisplayName = "Lethal")        // > 45°C
};

// ─── Delegates ───────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCore_OnStatChanged,
    FName, StatName, float, NewValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCore_OnSurvivalStatusChanged,
    ECore_SurvivalStatus, OldStatus, ECore_SurvivalStatus, NewStatus);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCore_OnPlayerDeath);

// ─── SurvivalComponent ───────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent),
       BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

    // ── UActorComponent overrides ──────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // ── Core stat accessors ────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetHealth() const { return Health; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetHunger() const { return Hunger; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetThirst() const { return Thirst; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetStamina() const { return Stamina; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetBodyTemperature() const { return BodyTemperature; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetFear() const { return Fear; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    ECore_SurvivalStatus GetCurrentStatus() const { return CurrentStatus; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsAlive() const { return CurrentStatus != ECore_SurvivalStatus::Dead; }

    // ── Stat modifiers ─────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyDamage(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void HealHealth(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeFood(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void DrinkWater(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UseStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void RegenerateStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetAmbientTemperature(float CelsiusDegrees);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddFear(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ReduceFear(float Amount);

    // ── Debug ──────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Survival|Debug")
    void PrintSurvivalStats() const;

    // ── Delegates ──────────────────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FCore_OnStatChanged OnStatChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FCore_OnSurvivalStatusChanged OnSurvivalStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FCore_OnPlayerDeath OnPlayerDeath;

protected:
    // ── Survival stats ─────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats",
              meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats",
              meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats",
              meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Thirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats",
              meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats",
              meta = (ClampMin = "30.0", ClampMax = "45.0"))
    float BodyTemperature = 37.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats",
              meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Fear = 0.0f;

    // ── Drain rates (per second) ───────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HungerDrainRate = 0.5f;   // per second at rest

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float ThirstDrainRate = 0.8f;   // per second at rest

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaRegenRate = 5.0f;  // per second when not sprinting

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float FearDecayRate = 1.0f;     // per second when no threat

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StarvationDamageRate = 2.0f;  // HP/s when starving

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float DehydrationDamageRate = 3.0f; // HP/s when dehydrated

    // ── Thresholds ─────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float HungerWarningThreshold = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float ThirstWarningThreshold = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float StaminaExhaustionThreshold = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float CriticalHealthThreshold = 20.0f;

    // ── Ambient environment ────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|Environment")
    float AmbientTemperature = 28.0f;  // Celsius — set by BiomeManager

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|Environment")
    ECore_TemperatureZone TemperatureZone = ECore_TemperatureZone::Comfortable;

    // ── State ──────────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|State")
    ECore_SurvivalStatus CurrentStatus = ECore_SurvivalStatus::Healthy;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|State")
    bool bIsSprinting = false;

private:
    // ── Internal helpers ───────────────────────────────────────────────────
    void TickHunger(float DeltaTime);
    void TickThirst(float DeltaTime);
    void TickStamina(float DeltaTime);
    void TickTemperature(float DeltaTime);
    void TickFear(float DeltaTime);
    void EvaluateStatus();
    void BroadcastStatChange(FName StatName, float NewValue);
    void SetStatus(ECore_SurvivalStatus NewStatus);
    float ClampStat(float Value) const;
    ECore_TemperatureZone ClassifyTemperatureZone(float AmbientCelsius) const;
};
