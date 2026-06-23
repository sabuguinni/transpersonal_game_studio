#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "SurvivalComponent.generated.h"

// ---------------------------------------------------------------------------
// Enums — prefixed Core_ per Rule 2 (unique project-wide)
// ---------------------------------------------------------------------------

UENUM(BlueprintType)
enum class ECore_SurvivalStatus : uint8
{
    Healthy        UMETA(DisplayName = "Healthy"),
    Hungry         UMETA(DisplayName = "Hungry"),
    Thirsty        UMETA(DisplayName = "Thirsty"),
    Exhausted      UMETA(DisplayName = "Exhausted"),
    Hypothermic    UMETA(DisplayName = "Hypothermic"),
    Hyperthermic   UMETA(DisplayName = "Hyperthermic"),
    Critical       UMETA(DisplayName = "Critical"),
    Dead           UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECore_TemperatureZone : uint8
{
    Freezing   UMETA(DisplayName = "Freezing"),   // < -10 C
    Cold       UMETA(DisplayName = "Cold"),        // -10 to 5 C
    Cool       UMETA(DisplayName = "Cool"),        // 5 to 15 C
    Temperate  UMETA(DisplayName = "Temperate"),   // 15 to 28 C
    Warm       UMETA(DisplayName = "Warm"),        // 28 to 38 C
    Hot        UMETA(DisplayName = "Hot"),         // 38 to 50 C
    Extreme    UMETA(DisplayName = "Extreme")      // > 50 C
};

// ---------------------------------------------------------------------------
// Structs
// ---------------------------------------------------------------------------

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
    float MaxHunger = 100.f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Thirst = 100.f;       // 100 = hydrated, 0 = dehydrated

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float MaxThirst = 100.f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Stamina = 100.f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float MaxStamina = 100.f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Fear = 0.f;           // 0 = calm, 100 = panic

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float BodyTemperature = 37.f; // Celsius

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float AmbientTemperature = 22.f; // Celsius — set by BiomeManager
};

USTRUCT(BlueprintType)
struct FCore_SurvivalRates
{
    GENERATED_BODY()

    // Per-second drain rates (positive = draining)
    UPROPERTY(BlueprintReadWrite, Category = "Survival|Rates")
    float HungerDrainPerSecond = 0.05f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival|Rates")
    float ThirstDrainPerSecond = 0.08f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaDrainPerSecond = 5.0f;   // while sprinting

    UPROPERTY(BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaRegenPerSecond = 3.0f;   // while idle/walking

    UPROPERTY(BlueprintReadWrite, Category = "Survival|Rates")
    float HealthDrainFromStarvation = 0.5f; // per second when hunger=0

    UPROPERTY(BlueprintReadWrite, Category = "Survival|Rates")
    float HealthDrainFromDehydration = 0.8f; // per second when thirst=0

    UPROPERTY(BlueprintReadWrite, Category = "Survival|Rates")
    float FearDecayPerSecond = 2.0f;      // fear fades when safe

    UPROPERTY(BlueprintReadWrite, Category = "Survival|Rates")
    float BiomeTemperatureInfluence = 0.3f; // how fast ambient temp affects body temp
};

// ---------------------------------------------------------------------------
// Delegates
// ---------------------------------------------------------------------------

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCore_OnSurvivalStatusChanged,
    ECore_SurvivalStatus, OldStatus,
    ECore_SurvivalStatus, NewStatus);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCore_OnCharacterDied,
    AActor*, DeadActor);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCore_OnStatChanged,
    FName, StatName,
    float, NewValue);

// ---------------------------------------------------------------------------
// USurvivalComponent
// ---------------------------------------------------------------------------

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent),
       DisplayName = "Survival Component")
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

    // -----------------------------------------------------------------------
    // UActorComponent overrides
    // -----------------------------------------------------------------------
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // -----------------------------------------------------------------------
    // Public API — Stat Queries
    // -----------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetHealth() const { return Stats.Health; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetHunger() const { return Stats.Hunger; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetHungerPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetThirst() const { return Stats.Thirst; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetThirstPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetStamina() const { return Stats.Stamina; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetStaminaPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetFear() const { return Stats.Fear; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    ECore_SurvivalStatus GetCurrentStatus() const { return CurrentStatus; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsAlive() const { return CurrentStatus != ECore_SurvivalStatus::Dead; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    ECore_TemperatureZone GetTemperatureZone() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    FCore_SurvivalStats GetAllStats() const { return Stats; }

    // -----------------------------------------------------------------------
    // Public API — Stat Modifiers
    // -----------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyDamage(float Amount, AActor* DamageCauser = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void HealHealth(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeFood(float NutritionValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void DrinkWater(float HydrationValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void DrainStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void RestoreStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddFear(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ReduceFear(float Amount);

    // -----------------------------------------------------------------------
    // Public API — Environment Integration
    // -----------------------------------------------------------------------

    /** Called by BiomeManager to set current ambient temperature */
    UFUNCTION(BlueprintCallable, Category = "Survival|Environment")
    void SetAmbientTemperature(float CelsiusTemperature);

    /** Called by BiomeManager to apply biome survival modifiers */
    UFUNCTION(BlueprintCallable, Category = "Survival|Environment")
    void ApplyBiomeModifiers(float HungerMultiplier, float ThirstMultiplier, float StaminaMultiplier);

    /** True when character is sprinting — affects stamina drain */
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetSprinting(bool bIsSprinting);

    // -----------------------------------------------------------------------
    // Delegates
    // -----------------------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FCore_OnSurvivalStatusChanged OnSurvivalStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FCore_OnCharacterDied OnCharacterDied;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FCore_OnStatChanged OnStatChanged;

    // -----------------------------------------------------------------------
    // Configuration (editable in Blueprint/Details panel)
    // -----------------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    FCore_SurvivalStats InitialStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    FCore_SurvivalRates Rates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    bool bEnableSurvivalTick = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float TickIntervalSeconds = 1.0f; // How often survival drains update (performance)

private:
    // -----------------------------------------------------------------------
    // Internal state
    // -----------------------------------------------------------------------

    UPROPERTY()
    FCore_SurvivalStats Stats;

    UPROPERTY()
    ECore_SurvivalStatus CurrentStatus = ECore_SurvivalStatus::Healthy;

    bool bIsSprinting = false;

    // Biome multipliers (set by BiomeManager)
    float BiomeHungerMult = 1.0f;
    float BiomeThirstMult = 1.0f;
    float BiomeStaminaMult = 1.0f;

    // Accumulated time for tick interval
    float AccumulatedTime = 0.f;

    // -----------------------------------------------------------------------
    // Internal helpers
    // -----------------------------------------------------------------------

    void TickSurvivalStats(float DeltaTime);
    void UpdateTemperature(float DeltaTime);
    void UpdateStatus();
    void BroadcastStatChange(FName StatName, float NewValue);
    ECore_SurvivalStatus ComputeStatus() const;
};
