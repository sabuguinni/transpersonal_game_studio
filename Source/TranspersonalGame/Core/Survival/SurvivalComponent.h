// SurvivalComponent.h — Transpersonal Game Studio
// Core Systems Programmer — Agent #03
// Survival stats component: hunger, thirst, stamina, health, fear
// Attaches to TranspersonalCharacter. Ticks every frame.
// National Geographic realism: no spiritual content.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

// Drain rates per second (realistic prehistoric survival pacing)
#define HUNGER_DRAIN_RATE   0.08f   // Full hunger depletes in ~208 seconds (~3.5 min)
#define THIRST_DRAIN_RATE   0.12f   // Full thirst depletes in ~138 seconds (~2.3 min)
#define STAMINA_REGEN_RATE  0.50f   // Stamina recovers at 0.5/sec when not sprinting
#define STAMINA_DRAIN_RATE  1.20f   // Stamina drains at 1.2/sec when sprinting
#define FEAR_DECAY_RATE     0.05f   // Fear decays slowly when no threat nearby

UENUM(BlueprintType)
enum class ECore_SurvivalStatus : uint8
{
    Healthy      UMETA(DisplayName = "Healthy"),
    Hungry       UMETA(DisplayName = "Hungry"),
    Starving     UMETA(DisplayName = "Starving"),
    Thirsty      UMETA(DisplayName = "Thirsty"),
    Dehydrated   UMETA(DisplayName = "Dehydrated"),
    Exhausted    UMETA(DisplayName = "Exhausted"),
    Critical     UMETA(DisplayName = "Critical"),
};

USTRUCT(BlueprintType)
struct FCore_SurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Health = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Hunger = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Thirst = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Stamina = 100.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Fear = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Temperature = 37.0f;  // Body temperature in Celsius
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSurvivalStatusChanged, ECore_SurvivalStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatChanged, FName, StatName, float, NewValue);

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

    // --- Core Tick ---
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Stat Accessors ---
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
    FCore_SurvivalStats GetAllStats() const { return Stats; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    ECore_SurvivalStatus GetCurrentStatus() const { return CurrentStatus; }

    // --- Stat Modifiers ---
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeFood(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void DrinkWater(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void TakeDamage_Survival(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddFear(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetSprinting(bool bIsSprinting);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsDead() const { return Stats.Health <= 0.0f; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool CanSprint() const { return Stats.Stamina > 10.0f; }

    // --- Events ---
    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnSurvivalStatusChanged OnStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
    FOnStatChanged OnStatChanged;

    // --- Config ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float HungerDrainRate = HUNGER_DRAIN_RATE;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float ThirstDrainRate = THIRST_DRAIN_RATE;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float StaminaRegenRate = STAMINA_REGEN_RATE;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float StaminaDrainRate = STAMINA_DRAIN_RATE;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float FearDecayRate = FEAR_DECAY_RATE;

    // --- Starvation/Dehydration damage ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float StarvationDamageRate = 0.5f;   // HP/sec when hunger = 0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float DehydrationDamageRate = 1.0f;  // HP/sec when thirst = 0

private:
    UPROPERTY(VisibleAnywhere, Category = "Survival|State", meta = (AllowPrivateAccess = "true"))
    FCore_SurvivalStats Stats;

    UPROPERTY(VisibleAnywhere, Category = "Survival|State", meta = (AllowPrivateAccess = "true"))
    ECore_SurvivalStatus CurrentStatus;

    UPROPERTY(VisibleAnywhere, Category = "Survival|State", meta = (AllowPrivateAccess = "true"))
    bool bIsSprinting = false;

    void TickHunger(float DeltaTime);
    void TickThirst(float DeltaTime);
    void TickStamina(float DeltaTime);
    void TickFear(float DeltaTime);
    void TickStarvationDamage(float DeltaTime);
    void UpdateStatus();
    void ClampStats();
};
