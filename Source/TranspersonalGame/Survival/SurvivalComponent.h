// SurvivalComponent.h
// Performance Optimizer — Agent #04 | PROD_CYCLE_AUTO_20260624_003
// Survival stat component: hunger, thirst, temperature, stamina, fear.
// Attaches to ACharacter (player or NPC). Pure biological survival — no spiritual content.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

// ---------------------------------------------------------------------------
// Delegates — broadcast to Blueprint/UI
// ---------------------------------------------------------------------------
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPerf_OnStatLow,  float, CurrentValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPerf_OnSurvivalEvent);

// ---------------------------------------------------------------------------
// USurvivalComponent
// ---------------------------------------------------------------------------
UCLASS(ClassGroup=(Survival), meta=(BlueprintSpawnableComponent), DisplayName="Survival Component")
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // -----------------------------------------------------------------------
    // Core stat update methods (called every tick)
    // -----------------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category="Survival")
    void UpdateHunger(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category="Survival")
    void UpdateThirst(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category="Survival")
    void UpdateTemperature(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category="Survival")
    void UpdateStamina(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category="Survival")
    void UpdateFear(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category="Survival")
    void ApplyDamageFromStats(float DeltaTime);

    // -----------------------------------------------------------------------
    // Interaction methods
    // -----------------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category="Survival")
    void ConsumeFood(float Amount);

    UFUNCTION(BlueprintCallable, Category="Survival")
    void DrinkWater(float Amount);

    UFUNCTION(BlueprintCallable, Category="Survival")
    void SetAmbientTemperature(float NewAmbientTemp);

    UFUNCTION(BlueprintCallable, Category="Survival")
    void AddFear(float Amount);

    UFUNCTION(BlueprintCallable, Category="Survival")
    void ApplyHealthDamage(float Amount);

    UFUNCTION(BlueprintCallable, Category="Survival")
    void HealHealth(float Amount);

    // -----------------------------------------------------------------------
    // Accessors (percent 0..1 for HUD)
    // -----------------------------------------------------------------------
    UFUNCTION(BlueprintPure, Category="Survival")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category="Survival")
    float GetHungerPercent() const;

    UFUNCTION(BlueprintPure, Category="Survival")
    float GetThirstPercent() const;

    UFUNCTION(BlueprintPure, Category="Survival")
    float GetStaminaPercent() const;

    UFUNCTION(BlueprintPure, Category="Survival")
    float GetFearPercent() const;

    UFUNCTION(BlueprintPure, Category="Survival")
    bool IsStarving() const;

    UFUNCTION(BlueprintPure, Category="Survival")
    bool IsDehydrated() const;

    UFUNCTION(BlueprintPure, Category="Survival")
    bool IsExhausted() const;

    UFUNCTION(BlueprintPure, Category="Survival")
    bool IsInPanic() const;

    UFUNCTION(BlueprintPure, Category="Survival")
    bool IsDead() const;

    // -----------------------------------------------------------------------
    // Stats — exposed to Blueprint for HUD binding
    // -----------------------------------------------------------------------
    UPROPERTY(BlueprintReadOnly, Category="Survival|Stats")
    float Health;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Survival|Stats")
    float MaxHealth;

    UPROPERTY(BlueprintReadOnly, Category="Survival|Stats")
    float Hunger;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Survival|Stats")
    float MaxHunger;

    UPROPERTY(BlueprintReadOnly, Category="Survival|Stats")
    float Thirst;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Survival|Stats")
    float MaxThirst;

    UPROPERTY(BlueprintReadOnly, Category="Survival|Stats")
    float Temperature;

    UPROPERTY(BlueprintReadOnly, Category="Survival|Stats")
    float Stamina;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Survival|Stats")
    float MaxStamina;

    UPROPERTY(BlueprintReadOnly, Category="Survival|Stats")
    float Fear;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Survival|Stats")
    float MaxFear;

    // -----------------------------------------------------------------------
    // Tuning — editable in Blueprint defaults
    // -----------------------------------------------------------------------
    UPROPERTY(EditDefaultsOnly, Category="Survival|Rates")
    float HungerDrainRate;

    UPROPERTY(EditDefaultsOnly, Category="Survival|Rates")
    float ThirstDrainRate;

    UPROPERTY(EditDefaultsOnly, Category="Survival|Rates")
    float StaminaSprintDrain;

    UPROPERTY(EditDefaultsOnly, Category="Survival|Rates")
    float StaminaRestoreRate;

    UPROPERTY(EditDefaultsOnly, Category="Survival|Rates")
    float FearDecayRate;

    UPROPERTY(EditDefaultsOnly, Category="Survival|Rates")
    float FearBuildRate;

    UPROPERTY(EditDefaultsOnly, Category="Survival|Rates")
    float PredatorDetectRadius;

    UPROPERTY(EditDefaultsOnly, Category="Survival|Rates")
    float StarvationDamageRate;

    UPROPERTY(EditDefaultsOnly, Category="Survival|Rates")
    float DehydrationDamageRate;

    UPROPERTY(EditDefaultsOnly, Category="Survival|Rates")
    float HypothermiaDamageRate;

    UPROPERTY(EditDefaultsOnly, Category="Survival|Rates")
    float HyperthermaDamageRate;

    UPROPERTY(EditDefaultsOnly, Category="Survival|Temperature")
    float ComfortTempMin;

    UPROPERTY(EditDefaultsOnly, Category="Survival|Temperature")
    float ComfortTempMax;

    UPROPERTY(EditDefaultsOnly, Category="Survival|Temperature")
    float AmbientTemperature;

    UPROPERTY(EditDefaultsOnly, Category="Survival|Temperature")
    float TempChangeRate;

    // -----------------------------------------------------------------------
    // Delegates — bind in Blueprint or C++ for UI/gameplay reactions
    // -----------------------------------------------------------------------
    UPROPERTY(BlueprintAssignable, Category="Survival|Events")
    FPerf_OnStatLow OnHungerLow;

    UPROPERTY(BlueprintAssignable, Category="Survival|Events")
    FPerf_OnStatLow OnThirstLow;

    UPROPERTY(BlueprintAssignable, Category="Survival|Events")
    FPerf_OnSurvivalEvent OnStaminaExhausted;

    UPROPERTY(BlueprintAssignable, Category="Survival|Events")
    FPerf_OnSurvivalEvent OnPanicTriggered;

    UPROPERTY(BlueprintAssignable, Category="Survival|Events")
    FPerf_OnSurvivalEvent OnDeath;

private:
    UPROPERTY()
    ACharacter* OwnerCharacter;

    bool bIsSprinting;
    bool bIsDead;
};
