// SurvivalComponent.h
// Priority 2: Core survival mechanics for the transpersonal survival game
// Tracks Hunger, Thirst, Health, and Stamina for the player character
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatChanged, float, NewValue, float, MaxValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDeath);

UCLASS(ClassGroup=(Survival), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

    //--- Delegates ---
    UPROPERTY(BlueprintAssignable, Category="Survival")
    FOnStatChanged OnHungerChanged;

    UPROPERTY(BlueprintAssignable, Category="Survival")
    FOnStatChanged OnThirstChanged;

    UPROPERTY(BlueprintAssignable, Category="Survival")
    FOnStatChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category="Survival")
    FOnStatChanged OnStaminaChanged;

    UPROPERTY(BlueprintAssignable, Category="Survival")
    FOnPlayerDeath OnPlayerDeath;

    //--- Stats ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survival|Health")
    float MaxHealth = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survival|Hunger")
    float MaxHunger = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survival|Thirst")
    float MaxThirst = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survival|Stamina")
    float MaxStamina = 100.f;

    //--- Drain rates (per second) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survival|Rates")
    float HungerDrainRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survival|Rates")
    float ThirstDrainRate = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survival|Rates")
    float StaminaDrainRate = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survival|Rates")
    float StaminaRegenRate = 5.f;

    //--- Damage from starvation/dehydration (per second) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survival|Damage")
    float HungerDamageRate = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survival|Damage")
    float ThirstDamageRate = 2.f;

    //--- Blueprintable getters ---
    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Survival")
    float GetHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Survival")
    float GetHunger() const { return CurrentHunger; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Survival")
    float GetThirst() const { return CurrentThirst; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Survival")
    float GetStamina() const { return CurrentStamina; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="Survival")
    bool IsAlive() const { return bIsAlive; }

    //--- Actions ---
    UFUNCTION(BlueprintCallable, Category="Survival")
    void Eat(float Amount);

    UFUNCTION(BlueprintCallable, Category="Survival")
    void Drink(float Amount);

    UFUNCTION(BlueprintCallable, Category="Survival")
    void TakeDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category="Survival")
    void Heal(float HealAmount);

    UFUNCTION(BlueprintCallable, Category="Survival")
    void ConsumeStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category="Survival")
    bool HasEnoughStamina(float Amount) const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    UPROPERTY(VisibleAnywhere, Category="Survival|State")
    float CurrentHealth;

    UPROPERTY(VisibleAnywhere, Category="Survival|State")
    float CurrentHunger;

    UPROPERTY(VisibleAnywhere, Category="Survival|State")
    float CurrentThirst;

    UPROPERTY(VisibleAnywhere, Category="Survival|State")
    float CurrentStamina;

    UPROPERTY(VisibleAnywhere, Category="Survival|State")
    bool bIsAlive = true;

    UPROPERTY(VisibleAnywhere, Category="Survival|State")
    bool bIsRunning = false;

    void DrainStats(float DeltaTime);
    void ApplyStarvationDamage(float DeltaTime);
    void Die();
};
