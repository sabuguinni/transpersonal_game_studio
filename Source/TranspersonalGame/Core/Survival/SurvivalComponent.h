// SurvivalComponent.h
// Core Systems Programmer #03 — P3 Character System
// Survival stats: health, hunger, thirst, stamina, temperature, fear
// Integrated into TranspersonalCharacter via CreateDefaultSubobject

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
    Terrified   UMETA(DisplayName = "Terrified"),
    Critical    UMETA(DisplayName = "Critical"),
    Dead        UMETA(DisplayName = "Dead")
};

USTRUCT(BlueprintType)
struct FCore_SurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Health = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float MaxHealth = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Hunger = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float MaxHunger = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Thirst = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float MaxThirst = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Stamina = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float MaxStamina = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Temperature = 37.0f; // Celsius — normal body temp

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Fear = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float MaxFear = 100.0f;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ─── Stats ───────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    FCore_SurvivalStats Stats;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    ECore_SurvivalStatus CurrentStatus;

    // ─── Drain rates (units per second) ──────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HungerDrainRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float ThirstDrainRate = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaDrainRate = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaRecoveryRate = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float FearDecayRate = 2.0f;

    // ─── Thresholds ───────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float HungerDamageThreshold = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float ThirstDamageThreshold = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float CriticalHealthThreshold = 20.0f;

    // ─── Public API ───────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyDamage(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Heal(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Eat(float NutritionValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Drink(float HydrationValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void DrainStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddFear(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsSprinting() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetSprinting(bool bSprint);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetHungerPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetThirstPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetStaminaPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetFearPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    ECore_SurvivalStatus GetCurrentStatus() const;

private:
    bool bIsSprinting = false;

    void UpdateStatus();
    void ApplyHungerDamage(float DeltaTime);
    void ApplyThirstDamage(float DeltaTime);
    void RecoverStamina(float DeltaTime);
    void DecayFear(float DeltaTime);
    void ClampStats();
};
