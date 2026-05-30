#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core_SurvivalPhysics.generated.h"

UENUM(BlueprintType)
enum class ECore_SurvivalState : uint8
{
    Healthy     UMETA(DisplayName = "Healthy"),
    Tired       UMETA(DisplayName = "Tired"),
    Hungry      UMETA(DisplayName = "Hungry"),
    Thirsty     UMETA(DisplayName = "Thirsty"),
    Injured     UMETA(DisplayName = "Injured"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_SurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Thirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Temperature = 37.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Fear = 0.0f;

    FCore_SurvivalStats()
    {
        Health = 100.0f;
        Hunger = 100.0f;
        Thirst = 100.0f;
        Stamina = 100.0f;
        Temperature = 37.0f;
        Fear = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_SurvivalPhysics : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_SurvivalPhysics();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core survival stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    FCore_SurvivalStats SurvivalStats;

    // Survival state
    UPROPERTY(BlueprintReadOnly, Category = "Survival State")
    ECore_SurvivalState CurrentState;

    // Physics integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration")
    float MovementSpeedMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration")
    float JumpHeightMultiplier = 1.0f;

    // Survival mechanics
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UpdateSurvivalStats(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ApplyDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void RestoreHealth(float HealAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeFood(float FoodValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void DrinkWater(float WaterValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void RestoreStamina(float StaminaAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void SetFearLevel(float NewFearLevel);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    ECore_SurvivalState GetSurvivalState() const;

    // Physics effects based on survival state
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void ApplySurvivalPhysicsEffects();

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    float GetCurrentMovementSpeed() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    float GetCurrentJumpHeight() const;

private:
    // Internal calculations
    void CalculateSurvivalState();
    void UpdatePhysicsMultipliers();
    void HandleCriticalState();

    // Decay rates
    UPROPERTY(EditAnywhere, Category = "Survival Settings")
    float HungerDecayRate = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Survival Settings")
    float ThirstDecayRate = 0.8f;

    UPROPERTY(EditAnywhere, Category = "Survival Settings")
    float StaminaRecoveryRate = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Survival Settings")
    float FearDecayRate = 1.0f;

    // Physics references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;
};

#include "Core_SurvivalPhysics.generated.h"