#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

/**
 * SurvivalComponent — Core survival stats for the player character.
 * Tracks Health, Hunger, Thirst, Stamina, Temperature, and Fear.
 * Designed for ATranspersonalCharacter. Ticks every second to drain stats.
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── Stat Accessors ──────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetHealth() const { return Health; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetHunger() const { return Hunger; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetThirst() const { return Thirst; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetStamina() const { return Stamina; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetTemperature() const { return Temperature; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetFear() const { return Fear; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsAlive() const { return Health > 0.0f; }

    // ── Stat Modifiers ──────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyDamage(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Eat(float NutritionValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Drink(float HydrationValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void RecoverStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddFear(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ReduceFear(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetAmbientTemperature(float AmbientTemp);

    // ── Drain Rates (editable in Blueprint/Editor) ──────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HungerDrainPerSecond = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float ThirstDrainPerSecond = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaRecoveryPerSecond = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float FearDecayPerSecond = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float StarvationDamagePerSecond = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float DehydrationDamagePerSecond = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float HypothermiaDamagePerSecond = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float HyperthermiaThreshold = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
    float HypothermiaThreshold = 5.0f;

    // ── Delegates ───────────────────────────────────────────────────────────

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnDeath OnDeath;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatChanged, float, NewValue);
    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnStatChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnStatChanged OnHungerChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnStatChanged OnThirstChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnStatChanged OnStaminaChanged;

private:
    // ── Core Stats ──────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|Stats", meta = (AllowPrivateAccess = "true"))
    float Health = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|Stats", meta = (AllowPrivateAccess = "true"))
    float Hunger = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|Stats", meta = (AllowPrivateAccess = "true"))
    float Thirst = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|Stats", meta = (AllowPrivateAccess = "true"))
    float Stamina = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|Stats", meta = (AllowPrivateAccess = "true"))
    float Temperature = 20.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|Stats", meta = (AllowPrivateAccess = "true"))
    float Fear = 0.0f;

    // ── Internal State ──────────────────────────────────────────────────────

    bool bIsDead = false;
    float AccumulatedTime = 0.0f;

    // ── Internal Helpers ────────────────────────────────────────────────────

    void TickSurvivalStats(float DeltaTime);
    void ClampStat(float& Stat, float Min = 0.0f, float Max = 100.0f);
    void HandleDeath();
};
