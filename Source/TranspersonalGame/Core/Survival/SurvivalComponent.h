#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "World/BiomeManager.h"
#include "SurvivalComponent.generated.h"

// ── Survival stat identifiers ────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ECore_SurvivalStat : uint8
{
    Health      UMETA(DisplayName = "Health"),
    Hunger      UMETA(DisplayName = "Hunger"),
    Thirst      UMETA(DisplayName = "Thirst"),
    Stamina     UMETA(DisplayName = "Stamina"),
    Temperature UMETA(DisplayName = "Temperature"),
    Fear        UMETA(DisplayName = "Fear"),
};

// ── Per-stat runtime data ────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FCore_StatValue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Current = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Max = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float DecayRatePerSecond = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float CriticalThreshold = 20.f;

    float GetNormalized() const { return (Max > 0.f) ? FMath::Clamp(Current / Max, 0.f, 1.f) : 0.f; }
    bool  IsCritical()    const { return Current <= CriticalThreshold; }
    bool  IsDepleted()    const { return Current <= 0.f; }
};

// ── Delegate signatures ──────────────────────────────────────────────────────
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatChanged,
    ECore_SurvivalStat, Stat, float, NewValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatCritical,
    ECore_SurvivalStat, Stat);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterDied);

/**
 * USurvivalComponent
 *
 * Manages all survival statistics for the prehistoric human player:
 * Health, Hunger, Thirst, Stamina, Temperature, Fear.
 *
 * Biome integration: reads FEng_BiomeConfig from ABiomeManager to apply
 * temperature and humidity modifiers to decay rates each tick.
 *
 * Attach to ATranspersonalCharacter in its constructor.
 */
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent),
       DisplayName = "Survival Component")
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

    // ── Lifecycle ────────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // ── Stat accessors ───────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetStat(ECore_SurvivalStat Stat) const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetStatNormalized(ECore_SurvivalStat Stat) const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsStatCritical(ECore_SurvivalStat Stat) const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsAlive() const { return !bIsDead; }

    // ── Stat modifiers ───────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyDamage(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Heal(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeFood(float NutritionValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void DrinkWater(float HydrationValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void RestoreStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddFear(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ReduceFear(float Amount);

    // ── Biome integration ────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival|Biome")
    void ApplyBiomeModifiers(const FEng_BiomeConfig& BiomeConfig);

    UFUNCTION(BlueprintCallable, Category = "Survival|Biome")
    void SetCurrentBiome(EEng_BiomeType NewBiome);

    // ── Sprint / stamina drain ───────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival|Stamina")
    void SetSprinting(bool bSprinting);

    UFUNCTION(BlueprintPure, Category = "Survival|Stamina")
    bool CanSprint() const;

    // ── Delegates ────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnStatChanged OnStatChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnStatCritical OnStatCritical;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnCharacterDied OnCharacterDied;

    // ── Editable defaults ────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats")
    FCore_StatValue Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats")
    FCore_StatValue Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats")
    FCore_StatValue Thirst;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats")
    FCore_StatValue Stamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats")
    FCore_StatValue Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats")
    FCore_StatValue Fear;

    /** Seconds between full survival tick updates */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config",
              meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float TickInterval = 1.0f;

    /** Stamina drain per second while sprinting */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float SprintStaminaDrain = 8.0f;

    /** Stamina regen per second while not sprinting */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float StaminaRegenRate = 4.0f;

    /** Minimum stamina required to start sprinting */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float SprintMinStamina = 10.0f;

    /** Health damage per second when hunger is depleted */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float StarvationDamageRate = 1.0f;

    /** Health damage per second when thirst is depleted */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float DehydrationDamageRate = 2.0f;

private:
    // Internal helpers
    void TickStat(FCore_StatValue& Stat, ECore_SurvivalStat StatType,
                  float DeltaTime, float DecayMultiplier = 1.0f);
    void HandleStatCritical(ECore_SurvivalStat Stat);
    void HandleDeath();
    FCore_StatValue& GetStatRef(ECore_SurvivalStat Stat);
    const FCore_StatValue& GetStatConstRef(ECore_SurvivalStat Stat) const;

    float AccumulatedTime = 0.f;
    bool  bIsDead         = false;
    bool  bIsSprinting    = false;

    // Biome modifier cache
    float BiomeTempModifier     = 1.0f;
    float BiomeHumidityModifier = 1.0f;
    EEng_BiomeType CurrentBiome = EEng_BiomeType::Plains;
};
