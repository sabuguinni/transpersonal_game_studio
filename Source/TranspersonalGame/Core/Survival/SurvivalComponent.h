// SurvivalComponent.h — Core Systems Programmer #03 — PROD_CYCLE_AUTO_20260702_005
// Survival stats component: health, hunger, thirst, stamina, fear, temperature.
// Attaches to TranspersonalCharacter. Integrates with BiomeManager for env modifiers.
// Realistic prehistoric survival — no spiritual/mystical content.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "SurvivalComponent.generated.h"

// ─── Delegates ───────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(
    FCore_OnSurvivalStatsUpdated,
    float, Health,
    float, Hunger,
    float, Thirst,
    float, Stamina,
    float, Fear,
    float, Temperature
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCore_OnPlayerDied);

// ─── SurvivalComponent ───────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // ─── Survival Stats (read-only from Blueprint) ────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats", meta = (AllowPrivateAccess = "true"))
    float Health;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats", meta = (AllowPrivateAccess = "true"))
    float MaxHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats", meta = (AllowPrivateAccess = "true"))
    float Hunger;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats", meta = (AllowPrivateAccess = "true"))
    float MaxHunger;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats", meta = (AllowPrivateAccess = "true"))
    float Thirst;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats", meta = (AllowPrivateAccess = "true"))
    float MaxThirst;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats", meta = (AllowPrivateAccess = "true"))
    float Stamina;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats", meta = (AllowPrivateAccess = "true"))
    float MaxStamina;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats", meta = (AllowPrivateAccess = "true"))
    float Fear;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats", meta = (AllowPrivateAccess = "true"))
    float MaxFear;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats", meta = (AllowPrivateAccess = "true"))
    float Temperature;

    // ─── Drain Rates (editable in editor) ────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HungerDrainRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float ThirstDrainRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaDrainRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaRegenRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float FearDecayRate;

    // ─── Damage Thresholds ────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float StarvationDamageRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float DehydrationDamageRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float HypothermiaDamageRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float HyperthermiaThreshold;

    // ─── State Flags ──────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Survival|State")
    bool bIsExerting;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|State")
    bool bIsResting;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|State")
    bool bIsSprinting;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|State")
    bool bIsStarving;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|State")
    bool bIsDehydrated;

    // ─── Biome Conditions (updated by BiomeManager) ───────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Biome")
    float CurrentBiomeTemperature;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Biome")
    float CurrentBiomeDanger;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Biome")
    float CurrentBiomeHumidity;

    // ─── Tick Config ──────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float TickIntervalSeconds;

    // ─── Delegates ────────────────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FCore_OnSurvivalStatsUpdated OnSurvivalStatsUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FCore_OnPlayerDied OnPlayerDied;

    // ─── Public API (callable from Blueprint and C++) ─────────────────────

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyHealthDamage(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void HealHealth(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeFood(float NutritionValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeWater(float HydrationValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddFear(float FearAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ReduceFear(float FearAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetExerting(bool bExerting);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetSprinting(bool bSprinting);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetResting(bool bResting);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UpdateBiomeConditions(float BiomeTemperature, float BiomeDanger, float BiomeHumidity);

    // ─── Stat Accessors ───────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival|Stats")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival|Stats")
    float GetHungerPercent() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival|Stats")
    float GetThirstPercent() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival|Stats")
    float GetStaminaPercent() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival|Stats")
    float GetFearPercent() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival")
    bool IsInCriticalState() const;

private:
    FTimerHandle SurvivalTickHandle;

    UFUNCTION()
    void SurvivalTick();
};
