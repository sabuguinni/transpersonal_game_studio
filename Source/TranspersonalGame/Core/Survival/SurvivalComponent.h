// SurvivalComponent.h
// Core Systems Programmer #03 — Transpersonal Game Studio
// Survival stats component: health, hunger, thirst, stamina, body temperature
// Integrates with BiomeManager for environmental temperature modifiers
// Follows UE5 C++ Compilation Rules: USTRUCT at global scope, unique prefixes

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

// ── Enums (global scope — Rule 1) ────────────────────────────────────────────

UENUM(BlueprintType)
enum class ECore_SurvivalStatus : uint8
{
    Healthy        UMETA(DisplayName = "Healthy"),
    Hungry         UMETA(DisplayName = "Hungry"),
    Starving       UMETA(DisplayName = "Starving"),
    Thirsty        UMETA(DisplayName = "Thirsty"),
    Dehydrated     UMETA(DisplayName = "Dehydrated"),
    Hypothermic    UMETA(DisplayName = "Hypothermic"),
    Hyperthermic   UMETA(DisplayName = "Hyperthermic"),
    Exhausted      UMETA(DisplayName = "Exhausted"),
    Critical       UMETA(DisplayName = "Critical"),
    Dead           UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECore_DamageSource : uint8
{
    None           UMETA(DisplayName = "None"),
    Starvation     UMETA(DisplayName = "Starvation"),
    Dehydration    UMETA(DisplayName = "Dehydration"),
    Hypothermia    UMETA(DisplayName = "Hypothermia"),
    Hyperthermia   UMETA(DisplayName = "Hyperthermia"),
    DinosaurAttack UMETA(DisplayName = "DinosaurAttack"),
    FallDamage     UMETA(DisplayName = "FallDamage"),
    Drowning       UMETA(DisplayName = "Drowning"),
    Exhaustion     UMETA(DisplayName = "Exhaustion")
};

// ── Structs (global scope — Rule 1) ──────────────────────────────────────────

USTRUCT(BlueprintType)
struct FCore_SurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Hunger = 100.0f;          // 0 = starving, 100 = full

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxHunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Thirst = 100.0f;          // 0 = dehydrated, 100 = hydrated

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxThirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float BodyTemperature = 37.0f;  // Celsius — normal human core temp

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Fear = 0.0f;              // 0 = calm, 100 = panic

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxFear = 100.0f;

    FCore_SurvivalStats()
        : Health(100.f), MaxHealth(100.f)
        , Hunger(100.f), MaxHunger(100.f)
        , Thirst(100.f), MaxThirst(100.f)
        , Stamina(100.f), MaxStamina(100.f)
        , BodyTemperature(37.f)
        , Fear(0.f), MaxFear(100.f)
    {}
};

USTRUCT(BlueprintType)
struct FCore_SurvivalRates
{
    GENERATED_BODY()

    /** Hunger drain per second at rest */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HungerDrainRate = 0.5f;

    /** Hunger drain multiplier when sprinting */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HungerSprintMultiplier = 2.5f;

    /** Thirst drain per second at rest */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float ThirstDrainRate = 0.8f;

    /** Thirst drain multiplier in hot biomes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float ThirstHeatMultiplier = 2.0f;

    /** Stamina drain per second while sprinting */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaDrainRate = 15.0f;

    /** Stamina recovery per second while idle */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaRecoveryRate = 8.0f;

    /** Health drain per second when starving */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StarvationDamageRate = 1.0f;

    /** Health drain per second when dehydrated */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float DehydrationDamageRate = 2.0f;

    /** Health drain per second in hypothermia */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HypothermiaDamageRate = 1.5f;

    /** Health drain per second in hyperthermia */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HyperthermaDamageRate = 1.5f;

    /** Fear decay per second when no threats visible */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float FearDecayRate = 5.0f;

    FCore_SurvivalRates()
        : HungerDrainRate(0.5f), HungerSprintMultiplier(2.5f)
        , ThirstDrainRate(0.8f), ThirstHeatMultiplier(2.0f)
        , StaminaDrainRate(15.0f), StaminaRecoveryRate(8.0f)
        , StarvationDamageRate(1.0f), DehydrationDamageRate(2.0f)
        , HypothermiaDamageRate(1.5f), HyperthermaDamageRate(1.5f)
        , FearDecayRate(5.0f)
    {}
};

// ── Component Declaration ─────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

    // ── UActorComponent overrides ─────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── Survival Stats (exposed to Blueprint) ────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    FCore_SurvivalStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    FCore_SurvivalRates Rates;

    // ── Status Query ──────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival")
    ECore_SurvivalStatus GetCurrentStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsAlive() const { return Stats.Health > 0.0f; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsSprinting() const { return bIsSprinting; }

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

    // ── Modifiers ─────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyDamage(float Amount, ECore_DamageSource Source);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Eat(float NutritionValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Drink(float HydrationValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetSprinting(bool bSprint);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddFear(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ReduceFear(float Amount);

    /** Called by BiomeManager to set ambient temperature at player location */
    UFUNCTION(BlueprintCallable, Category = "Survival|Biome")
    void SetAmbientTemperature(float AmbientCelsius);

    /** Get the current ambient temperature affecting this character */
    UFUNCTION(BlueprintCallable, Category = "Survival|Biome")
    float GetAmbientTemperature() const { return AmbientTemperature; }

    // ── Events (override in Blueprint) ───────────────────────────────────
    UFUNCTION(BlueprintImplementableEvent, Category = "Survival|Events")
    void OnDeath(ECore_DamageSource Cause);

    UFUNCTION(BlueprintImplementableEvent, Category = "Survival|Events")
    void OnStatusChanged(ECore_SurvivalStatus NewStatus);

    UFUNCTION(BlueprintImplementableEvent, Category = "Survival|Events")
    void OnStaminaDepleted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Survival|Events")
    void OnHungerCritical();

    UFUNCTION(BlueprintImplementableEvent, Category = "Survival|Events")
    void OnThirstCritical();

private:
    // ── Internal state ────────────────────────────────────────────────────
    bool bIsSprinting = false;
    bool bIsDead = false;
    float AmbientTemperature = 25.0f;   // Default: warm Cretaceous day
    ECore_SurvivalStatus LastStatus = ECore_SurvivalStatus::Healthy;

    // ── Tick helpers ──────────────────────────────────────────────────────
    void TickHunger(float DeltaTime);
    void TickThirst(float DeltaTime);
    void TickStamina(float DeltaTime);
    void TickBodyTemperature(float DeltaTime);
    void TickFear(float DeltaTime);
    void TickEnvironmentalDamage(float DeltaTime);
    void CheckStatusChange();

    /** Clamp a value between 0 and Max */
    static float ClampStat(float Value, float Max) { return FMath::Clamp(Value, 0.0f, Max); }
};
