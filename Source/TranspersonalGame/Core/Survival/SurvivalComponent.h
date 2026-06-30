// SurvivalComponent.h
// Core Systems Programmer #03 — Transpersonal Game Studio
// Survival stats component: health, hunger, thirst, stamina, fear
// Attaches to TranspersonalCharacter. Ticks every second to drain stats.
// Biome integration: GetBiomeAtLocation() modulates drain rates.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ECore_SurvivalStat : uint8
{
    Health    UMETA(DisplayName = "Health"),
    Hunger    UMETA(DisplayName = "Hunger"),
    Thirst    UMETA(DisplayName = "Thirst"),
    Stamina   UMETA(DisplayName = "Stamina"),
    Fear      UMETA(DisplayName = "Fear")
};

UENUM(BlueprintType)
enum class ECore_SurvivalState : uint8
{
    Healthy     UMETA(DisplayName = "Healthy"),
    Hungry      UMETA(DisplayName = "Hungry"),
    Starving    UMETA(DisplayName = "Starving"),
    Thirsty     UMETA(DisplayName = "Thirsty"),
    Dehydrated  UMETA(DisplayName = "Dehydrated"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Panicking   UMETA(DisplayName = "Panicking"),
    Dead        UMETA(DisplayName = "Dead")
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FCore_SurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Thirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxHunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxThirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxFear = 100.0f;
};

USTRUCT(BlueprintType)
struct FCore_DrainRates
{
    GENERATED_BODY()

    /** Health drain per second when starving/dehydrated */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drain Rates")
    float HealthDrainStarving = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drain Rates")
    float HealthDrainDehydrated = 0.8f;

    /** Hunger drain per second at rest */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drain Rates")
    float HungerDrainRest = 0.2f;

    /** Hunger drain per second while sprinting */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drain Rates")
    float HungerDrainSprint = 0.6f;

    /** Thirst drain per second at rest */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drain Rates")
    float ThirstDrainRest = 0.3f;

    /** Thirst drain per second while sprinting */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drain Rates")
    float ThirstDrainSprint = 0.9f;

    /** Stamina drain per second while sprinting */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drain Rates")
    float StaminaDrainSprint = 5.0f;

    /** Stamina recovery per second at rest */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drain Rates")
    float StaminaRecoveryRest = 3.0f;

    /** Fear drain per second when no threats nearby */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drain Rates")
    float FearDecayRate = 1.0f;

    /** Biome temperature multiplier on thirst drain (hot biomes = faster thirst) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drain Rates")
    float TemperatureThirstMultiplier = 1.0f;
};

// ─── Delegates ────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCore_OnStatChanged,
    ECore_SurvivalStat, Stat, float, NewValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCore_OnSurvivalStateChanged,
    ECore_SurvivalState, NewState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCore_OnPlayerDied);

// ─── Class ────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Survival), meta = (BlueprintSpawnableComponent),
       DisplayName = "Survival Component")
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

    // ── Lifecycle ──────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // ── Stat Accessors ─────────────────────────────────────────────────────
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
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetHungerPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetThirstPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetStaminaPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetFearPercent() const;

    UFUNCTION(BlueprintPure, Category = "Survival")
    FCore_SurvivalStats GetAllStats() const { return Stats; }

    UFUNCTION(BlueprintPure, Category = "Survival")
    ECore_SurvivalState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Survival")
    bool IsAlive() const { return bIsAlive; }

    // ── Stat Modifiers ─────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyDamage(float Amount, AActor* DamageCauser = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void HealHealth(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeFood(float NutritionValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void DrinkWater(float HydrationValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddFear(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ReduceFear(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void RestoreStamina(float Amount);

    // ── Sprint Control ─────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetSprinting(bool bSprinting);

    UFUNCTION(BlueprintPure, Category = "Survival")
    bool IsSprinting() const { return bIsSprinting; }

    UFUNCTION(BlueprintPure, Category = "Survival")
    bool CanSprint() const;

    // ── Biome Integration ──────────────────────────────────────────────────
    /** Called by BiomeManager when player enters a new biome.
     *  Temperature modulates thirst drain. Humidity modulates health recovery. */
    UFUNCTION(BlueprintCallable, Category = "Survival|Biome")
    void OnBiomeChanged(float NewTemperature, float NewHumidity);

    // ── Debug ──────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Survival|Debug")
    void PrintSurvivalStats() const;

    // ── Delegates ──────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FCore_OnStatChanged OnStatChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FCore_OnSurvivalStateChanged OnSurvivalStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FCore_OnPlayerDied OnPlayerDied;

    // ── Configuration ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    FCore_SurvivalStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    FCore_DrainRates DrainRates;

    /** How often (seconds) survival tick runs. Default 1.0 = every second. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config",
              meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float SurvivalTickInterval = 1.0f;

    /** Hunger threshold below which health starts draining */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config",
              meta = (ClampMin = "0.0", ClampMax = "50.0"))
    float StarvationThreshold = 10.0f;

    /** Thirst threshold below which health starts draining faster */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config",
              meta = (ClampMin = "0.0", ClampMax = "50.0"))
    float DehydrationThreshold = 10.0f;

    /** Stamina threshold below which sprinting is disabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config",
              meta = (ClampMin = "0.0", ClampMax = "30.0"))
    float ExhaustionThreshold = 5.0f;

    /** Fear threshold above which movement speed is reduced */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config",
              meta = (ClampMin = "50.0", ClampMax = "100.0"))
    float PanicThreshold = 80.0f;

private:
    void TickSurvival(float DeltaTime);
    void UpdateSurvivalState();
    void BroadcastStatChange(ECore_SurvivalStat Stat, float NewValue);
    void ClampStat(float& Stat, float Min, float Max);

    UPROPERTY()
    ECore_SurvivalState CurrentState = ECore_SurvivalState::Healthy;

    UPROPERTY()
    bool bIsAlive = true;

    UPROPERTY()
    bool bIsSprinting = false;

    float SurvivalTickAccumulator = 0.0f;

    // Biome-modulated multipliers (set by OnBiomeChanged)
    float BiomeThirstMultiplier = 1.0f;
    float BiomeHumidityHealBonus = 0.0f;
};
