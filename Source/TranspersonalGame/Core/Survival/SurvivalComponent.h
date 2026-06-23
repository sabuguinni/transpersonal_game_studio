// SurvivalComponent.h
// Core Systems Programmer #03 — P3 Character System
// Survival stats component: health, hunger, thirst, stamina, temperature, fear.
// Attach to TranspersonalCharacter. Ticks via internal timer (1s interval).

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Delegates — broadcast to HUD (#UI), Audio (#16), AI (#12)
// ─────────────────────────────────────────────────────────────────────────────
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSurv_OnStatChanged, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSurv_OnDeath);

// ─────────────────────────────────────────────────────────────────────────────
// USurvivalComponent
// ─────────────────────────────────────────────────────────────────────────────
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Internal timer callback
    void SurvivalTick();

    FTimerHandle SurvivalTickHandle;

public:
    // ── Current stat values ──────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats")
    float Health;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats")
    float MaxHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats")
    float Hunger;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats")
    float MaxHunger;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats")
    float Thirst;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats")
    float MaxThirst;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats")
    float Stamina;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats")
    float MaxStamina;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats")
    float Temperature;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats")
    float Fear;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|Stats")
    float MaxFear;

    // ── Drain / regen rates (editable in Blueprint/Details) ─────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HungerDrainRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float ThirstDrainRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaRegenRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaDrainRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float FearDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float SurvivalTickInterval;

    // ── State flags ──────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Survival|State")
    bool bIsSprinting;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|State")
    bool bIsDead;

    // ── Delegates ────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FSurv_OnStatChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FSurv_OnStatChanged OnHungerChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FSurv_OnStatChanged OnThirstChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FSurv_OnStatChanged OnStaminaChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FSurv_OnStatChanged OnFearChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FSurv_OnStatChanged OnTemperatureChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FSurv_OnDeath OnDeath;

    // ── Stat modifiers (Blueprint-callable) ──────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ModifyHealth(float Delta);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ModifyHunger(float Delta);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ModifyThirst(float Delta);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ModifyStamina(float Delta);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ModifyFear(float Delta);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetTemperature(float NewTemp);

    // ── Gameplay actions ─────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Eat(float NutritionValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Drink(float HydrationValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetSprinting(bool bSprinting);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool CanSprint() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsAlive() const;

    // ── Dinosaur encounter ───────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void OnDinosaurEncounter(float FearAmount, float DamageAmount);

    // ── Percent accessors for HUD ────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Survival|HUD")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Survival|HUD")
    float GetHungerPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Survival|HUD")
    float GetThirstPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Survival|HUD")
    float GetStaminaPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Survival|HUD")
    float GetFearPercent() const;
};
