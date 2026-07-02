// SurvivalComponent.h — Transpersonal Game Studio
// Core Systems Programmer — Agent #03
// Survival stats component: health, hunger, thirst, stamina, fear
// Attaches to TranspersonalCharacter for tick-based decay

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

UENUM(BlueprintType)
enum class ECore_SurvivalStatus : uint8
{
    Healthy     UMETA(DisplayName = "Healthy"),
    Hungry      UMETA(DisplayName = "Hungry"),
    Thirsty     UMETA(DisplayName = "Thirsty"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Panicked    UMETA(DisplayName = "Panicked"),
    Critical    UMETA(DisplayName = "Critical"),
    Dead        UMETA(DisplayName = "Dead")
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
    float Temperature = 37.0f;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Stat accessors ---
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
    ECore_SurvivalStatus GetStatus() const { return CurrentStatus; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    FCore_SurvivalStats GetAllStats() const { return Stats; }

    // --- Modifiers ---
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyDamage(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeFood(float NutritionValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void DrinkWater(float HydrationValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddFear(float FearAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ReduceFear(float FearAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SpendStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void RecoverStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsAlive() const { return Stats.Health > 0.0f; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsSprinting() const { return bIsSprinting; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetSprinting(bool bSprint) { bIsSprinting = bSprint; }

    // --- Decay rates (configurable per biome/difficulty) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HungerDecayRate = 1.5f;  // units per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float ThirstDecayRate = 2.0f;  // units per second (faster than hunger)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaDecayRate = 10.0f;  // units per second while sprinting

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float StaminaRecoveryRate = 5.0f;  // units per second while idle

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float FearDecayRate = 3.0f;  // units per second (fear fades naturally)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HealthDecayFromStarvation = 2.0f;  // damage per second when hunger=0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
    float HealthDecayFromDehydration = 3.0f;  // damage per second when thirst=0

private:
    UPROPERTY(VisibleAnywhere, Category = "Survival")
    FCore_SurvivalStats Stats;

    UPROPERTY(VisibleAnywhere, Category = "Survival")
    ECore_SurvivalStatus CurrentStatus;

    bool bIsSprinting = false;

    void UpdateStatus();
    void ApplyDecay(float DeltaTime);
};
