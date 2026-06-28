// SurvivalComponent.h
// Core Systems Programmer #03 — Transpersonal Game Studio
// Survival stats component: health, hunger, thirst, stamina, temperature, fear
// Attaches to TranspersonalCharacter and any NPC pawn that needs survival tracking.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

// ---------------------------------------------------------------------------
// Enums — must be at global scope (UE5 compilation rule)
// ---------------------------------------------------------------------------

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

UENUM(BlueprintType)
enum class ECore_SurvivalState : uint8
{
    Healthy     UMETA(DisplayName = "Healthy"),
    Hungry      UMETA(DisplayName = "Hungry"),
    Starving    UMETA(DisplayName = "Starving"),
    Dehydrated  UMETA(DisplayName = "Dehydrated"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Hypothermic UMETA(DisplayName = "Hypothermic"),
    Hyperthermic UMETA(DisplayName = "Hyperthermic"),
    Panicked    UMETA(DisplayName = "Panicked"),
    Dead        UMETA(DisplayName = "Dead"),
};

// ---------------------------------------------------------------------------
// Delegates
// ---------------------------------------------------------------------------

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatChanged, ECore_SurvivalStat, Stat, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSurvivalStateChanged, ECore_SurvivalState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterDied);

// ---------------------------------------------------------------------------
// USurvivalComponent
// ---------------------------------------------------------------------------

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

    // UActorComponent overrides
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // -----------------------------------------------------------------------
    // Core Stats (0.0 - 100.0 range)
    // -----------------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats")
    float Hunger = 100.0f;  // 100 = full, 0 = starving

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats")
    float Thirst = 100.0f;  // 100 = hydrated, 0 = dehydrated

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats")
    float MaxStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats")
    float BodyTemperature = 37.0f;  // Celsius — normal 36-38

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Stats")
    float Fear = 0.0f;  // 0 = calm, 100 = full panic

    // -----------------------------------------------------------------------
    // Drain Rates (units per second)
    // -----------------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HungerDrainRate = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float ThirstDrainRate = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaDrainRate = 10.0f;  // while sprinting

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaRegenRate = 5.0f;   // while idle/walking

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float FearDecayRate = 2.0f;      // fear reduces over time when safe

    // -----------------------------------------------------------------------
    // State
    // -----------------------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, Category = "Survival|State")
    ECore_SurvivalState CurrentState = ECore_SurvivalState::Healthy;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|State")
    bool bIsSprinting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Survival|State")
    bool bIsDead = false;

    // -----------------------------------------------------------------------
    // Delegates
    // -----------------------------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnStatChanged OnStatChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnSurvivalStateChanged OnSurvivalStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnCharacterDied OnCharacterDied;

    // -----------------------------------------------------------------------
    // Blueprint-callable methods
    // -----------------------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Heal(float HealAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Eat(float NutritionValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Drink(float HydrationValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddFear(float FearAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetSprinting(bool bSprinting);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetStatValue(ECore_SurvivalStat Stat) const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsAlive() const { return !bIsDead; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    ECore_SurvivalState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetStaminaPercent() const;

private:
    void TickHunger(float DeltaTime);
    void TickThirst(float DeltaTime);
    void TickStamina(float DeltaTime);
    void TickFear(float DeltaTime);
    void TickStarvationDamage(float DeltaTime);
    void UpdateSurvivalState();
    void BroadcastStatChange(ECore_SurvivalStat Stat, float NewValue);
    void Die();

    float AccumulatedStarvationTime = 0.0f;
    float AccumulatedDehydrationTime = 0.0f;
};
