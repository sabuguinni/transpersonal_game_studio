// SurvivalComponent.h
// Core Systems Programmer — Agent #3
// Survival stats component: health, hunger, thirst, stamina, fear
// Attaches to TranspersonalCharacter. Ticks every 1s via timer (not per-frame).

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatChanged, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDied);

UCLASS(ClassGroup=(Survival), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // --- Stat accessors ---
    UFUNCTION(BlueprintCallable, Category="Survival")
    float GetHealth() const { return Health; }

    UFUNCTION(BlueprintCallable, Category="Survival")
    float GetHunger() const { return Hunger; }

    UFUNCTION(BlueprintCallable, Category="Survival")
    float GetThirst() const { return Thirst; }

    UFUNCTION(BlueprintCallable, Category="Survival")
    float GetStamina() const { return Stamina; }

    UFUNCTION(BlueprintCallable, Category="Survival")
    float GetFear() const { return Fear; }

    UFUNCTION(BlueprintCallable, Category="Survival")
    bool IsAlive() const { return bIsAlive; }

    // --- Stat modifiers ---
    UFUNCTION(BlueprintCallable, Category="Survival")
    void ApplyDamage(float Amount);

    UFUNCTION(BlueprintCallable, Category="Survival")
    void Heal(float Amount);

    UFUNCTION(BlueprintCallable, Category="Survival")
    void ConsumeFood(float Amount);

    UFUNCTION(BlueprintCallable, Category="Survival")
    void ConsumeWater(float Amount);

    UFUNCTION(BlueprintCallable, Category="Survival")
    void DrainStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category="Survival")
    void RestoreStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category="Survival")
    void AddFear(float Amount);

    UFUNCTION(BlueprintCallable, Category="Survival")
    void ReduceFear(float Amount);

    // --- Sprint support ---
    UFUNCTION(BlueprintCallable, Category="Survival")
    bool CanSprint() const { return Stamina > 5.0f && bIsAlive; }

    UFUNCTION(BlueprintCallable, Category="Survival")
    void SetSprinting(bool bSprinting);

    // --- Delegates ---
    UPROPERTY(BlueprintAssignable, Category="Survival")
    FOnStatChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category="Survival")
    FOnStatChanged OnHungerChanged;

    UPROPERTY(BlueprintAssignable, Category="Survival")
    FOnStatChanged OnThirstChanged;

    UPROPERTY(BlueprintAssignable, Category="Survival")
    FOnStatChanged OnStaminaChanged;

    UPROPERTY(BlueprintAssignable, Category="Survival")
    FOnStatChanged OnFearChanged;

    UPROPERTY(BlueprintAssignable, Category="Survival")
    FOnPlayerDied OnPlayerDied;

protected:
    // --- Stat values ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival|Stats", meta=(ClampMin="0.0", ClampMax="100.0"))
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival|Stats", meta=(ClampMin="0.0", ClampMax="100.0"))
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival|Stats", meta=(ClampMin="0.0", ClampMax="100.0"))
    float Thirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival|Stats", meta=(ClampMin="0.0", ClampMax="100.0"))
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival|Stats", meta=(ClampMin="0.0", ClampMax="100.0"))
    float Fear = 0.0f;

    // --- Drain rates (per second) ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival|Rates")
    float HungerDrainRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival|Rates")
    float ThirstDrainRate = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival|Rates")
    float StaminaRestoreRate = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival|Rates")
    float SprintStaminaDrainRate = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival|Rates")
    float FearDecayRate = 1.0f;

    // Damage per second when starving/dehydrated
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival|Rates")
    float StarvationDamageRate = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival|Rates")
    float DehydrationDamageRate = 3.0f;

    // Tick interval for survival timer
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Survival|Config")
    float TickInterval = 1.0f;

private:
    bool bIsAlive = true;
    bool bIsSprinting = false;

    FTimerHandle SurvivalTimerHandle;

    void SurvivalTick();
    void ClampStat(float& Stat);
    void TriggerDeath();
};
