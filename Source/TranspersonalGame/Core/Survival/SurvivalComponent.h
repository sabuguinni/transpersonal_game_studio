// SurvivalComponent.h — Survival stats component for TranspersonalCharacter
// Agent #3 — Core Systems Programmer | Cycle PROD_AUTO_20260627_005
// Tracks: Health, Hunger, Thirst, Stamina, Temperature, Fear

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

UENUM(BlueprintType)
enum class ECore_SurvivalState : uint8
{
    Healthy     UMETA(DisplayName = "Healthy"),
    Hungry      UMETA(DisplayName = "Hungry"),
    Thirsty     UMETA(DisplayName = "Thirsty"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Hypothermic UMETA(DisplayName = "Hypothermic"),
    Hyperthermic UMETA(DisplayName = "Hyperthermic"),
    Panicking   UMETA(DisplayName = "Panicking"),
    Dead        UMETA(DisplayName = "Dead")
};

USTRUCT(BlueprintType)
struct FCore_SurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Health = 100.f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float MaxHealth = 100.f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Hunger = 100.f;   // 100 = full, 0 = starving

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Thirst = 100.f;   // 100 = hydrated, 0 = dehydrated

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Stamina = 100.f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float MaxStamina = 100.f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float BodyTemperature = 37.f;   // Celsius

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Fear = 0.f;   // 0 = calm, 100 = panic
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

    // ── Tick ────────────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // ── Stats accessors ─────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetHealth() const { return Stats.Health; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetHunger() const { return Stats.Hunger; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetThirst() const { return Stats.Thirst; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetStamina() const { return Stats.Stamina; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetBodyTemperature() const { return Stats.BodyTemperature; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetFear() const { return Stats.Fear; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    ECore_SurvivalState GetCurrentState() const { return CurrentState; }

    // ── Modifiers ───────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyDamage(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Eat(float NutritionValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Drink(float HydrationValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void RestoreStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddFear(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ReduceFear(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsAlive() const { return Stats.Health > 0.f; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsExhausted() const { return Stats.Stamina <= 0.f; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsPanicking() const { return Stats.Fear >= 80.f; }

    // ── Drain rates (configurable from Blueprint/Editor) ────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HungerDrainPerSecond = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float ThirstDrainPerSecond = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaRegenPerSecond = 5.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float FearDecayPerSecond = 2.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StarvationDamagePerSecond = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float DehydrationDamagePerSecond = 2.f;

    // ── Full stats struct (exposed for Blueprint read) ───────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    FCore_SurvivalStats Stats;

private:
    UPROPERTY()
    ECore_SurvivalState CurrentState = ECore_SurvivalState::Healthy;

    void UpdateSurvivalState();
    void DrainStats(float DeltaTime);
    void ApplyPassiveEffects(float DeltaTime);

    bool bIsDead = false;
};
