// SurvivalComponent.h
// Core Systems Programmer #03 — Cycle AUTO_20260701_004
// Survival stats component: health, hunger, thirst, stamina, temperature, fear
// Attaches to TranspersonalCharacter (and any pawn that needs survival mechanics)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

UENUM(BlueprintType)
enum class ECore_SurvivalStatus : uint8
{
    Healthy     UMETA(DisplayName = "Healthy"),
    Hungry      UMETA(DisplayName = "Hungry"),
    Thirsty     UMETA(DisplayName = "Thirsty"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Hypothermic UMETA(DisplayName = "Hypothermic"),
    Hyperthermic UMETA(DisplayName = "Hyperthermic"),
    Critical    UMETA(DisplayName = "Critical"),
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
    float Hunger = 100.f;       // 100 = full, 0 = starving

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Thirst = 100.f;       // 100 = hydrated, 0 = dehydrated

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Stamina = 100.f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float MaxStamina = 100.f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Temperature = 37.f;   // Body temp in Celsius (normal = 37)

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Fear = 0.f;           // 0 = calm, 100 = panic
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
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
    float GetTemperature() const { return Stats.Temperature; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetFear() const { return Stats.Fear; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsAlive() const { return Stats.Health > 0.f; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    ECore_SurvivalStatus GetCurrentStatus() const;

    // ── Modifiers ───────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyDamage(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Heal(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeFood(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeWater(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void DrainStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void RestoreStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetFear(float NewFear);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetAmbientTemperature(float AmbientCelsius);

    // ── Config ──────────────────────────────────────────────────────────────

    /** How fast hunger drains per second (default: 0.5 units/s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float HungerDrainRate = 0.5f;

    /** How fast thirst drains per second (default: 0.8 units/s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float ThirstDrainRate = 0.8f;

    /** Stamina regen per second when not sprinting (default: 10 units/s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float StaminaRegenRate = 10.f;

    /** Health drain per second when starving (default: 1 unit/s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float StarvationDamageRate = 1.f;

    /** Health drain per second when dehydrated (default: 2 units/s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float DehydrationDamageRate = 2.f;

    /** Fear decay rate per second when no threat present (default: 5 units/s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float FearDecayRate = 5.f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    FCore_SurvivalStats Stats;

private:
    float AmbientTemperature = 25.f;   // degrees Celsius
    float TickAccumulator = 0.f;
    static constexpr float TickInterval = 1.0f; // process survival every 1 second

    void ProcessSurvivalTick(float DeltaSeconds);
    void ApplyTemperatureEffects(float DeltaSeconds);
};
