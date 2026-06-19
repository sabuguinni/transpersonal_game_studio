// SurvivalComponent.h
// Core Systems Programmer #03 — P3 Character System
// Survival stats component: health, hunger, thirst, stamina, fear, temperature
// Attaches to TranspersonalCharacter. Ticks every second to drain stats.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

UENUM(BlueprintType)
enum class ECore_SurvivalStat : uint8
{
    Health      UMETA(DisplayName = "Health"),
    Hunger      UMETA(DisplayName = "Hunger"),
    Thirst      UMETA(DisplayName = "Thirst"),
    Stamina     UMETA(DisplayName = "Stamina"),
    Fear        UMETA(DisplayName = "Fear"),
    Temperature UMETA(DisplayName = "Temperature"),
};

USTRUCT(BlueprintType)
struct FCore_SurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Health = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Hunger = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Thirst = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Stamina = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Fear = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Temperature = 37.f; // Celsius — normal body temp
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatChanged, ECore_SurvivalStat, Stat, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDied);

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── Query ──────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetStat(ECore_SurvivalStat Stat) const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetStatNormalized(ECore_SurvivalStat Stat) const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsCritical(ECore_SurvivalStat Stat) const;

    // ── Modification ───────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ModifyStat(ECore_SurvivalStat Stat, float Delta);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyDamage(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Eat(float NutritionValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Drink(float HydrationValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Rest(float StaminaRestored);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetFear(float FearLevel);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetAmbientTemperature(float CelsiusTemp);

    // ── Drain rates (editable per game balance) ────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HungerDrainPerSecond = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float ThirstDrainPerSecond = 0.08f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaDrainPerSecond = 0.0f; // Drained by movement, not time

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HealthDrainWhenStarving = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HealthDrainWhenDehydrated = 0.03f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float FearDecayPerSecond = 0.5f;

    // ── Thresholds ─────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float CriticalThreshold = 20.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float MaxTemperatureSafe = 42.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float MinTemperatureSafe = 32.f;

    // ── Events ─────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnStatChanged OnStatChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnPlayerDied OnPlayerDied;

    // ── State ──────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival")
    FCore_SurvivalStats Stats;

private:
    bool bIsDead = false;
    float AmbientTemperatureCelsius = 25.f;
    float AccumulatedTime = 0.f;

    void TickSurvivalDrain(float DeltaSeconds);
    void BroadcastStat(ECore_SurvivalStat Stat, float Value);
    void CheckDeath();
};
