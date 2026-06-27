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
    Critical    UMETA(DisplayName = "Critical"),
    Dead        UMETA(DisplayName = "Dead")
};

USTRUCT(BlueprintType)
struct FCore_SurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxHunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Thirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxThirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float BodyTemperature = 37.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Fear = 0.0f;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalComponent();

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
    ECore_SurvivalStatus GetSurvivalStatus() const;

    // --- Modifiers ---
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyDamage(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Eat(float NutritionValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Drink(float HydrationValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void Rest(float StaminaRestored);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddFear(float FearAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ReduceFear(float FearAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsAlive() const { return Stats.Health > 0.0f; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    bool IsSprinting() const { return bIsSprinting; }

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetSprinting(bool bSprint);

    // --- Config ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float HungerDecayRate = 2.0f;  // per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float ThirstDecayRate = 3.0f;  // per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float StaminaRecoveryRate = 10.0f;  // per second when not sprinting

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float StaminaDrainRate = 15.0f;  // per second when sprinting

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float FearDecayRate = 5.0f;  // per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float StarvationDamageRate = 1.0f;  // health lost per second when starving

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Config")
    float DehydrationDamageRate = 2.0f;  // health lost per second when dehydrated

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    FCore_SurvivalStats Stats;

private:
    bool bIsSprinting = false;
    float TimeSinceLastTick = 0.0f;

    void TickHunger(float DeltaTime);
    void TickThirst(float DeltaTime);
    void TickStamina(float DeltaTime);
    void TickFear(float DeltaTime);
    void TickStarvationDamage(float DeltaTime);
};
