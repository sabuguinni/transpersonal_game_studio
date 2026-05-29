#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SurvivalLocomotionSystem.generated.h"

// Locomotion states for survival gameplay
UENUM(BlueprintType)
enum class EAnim_SurvivalLocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Crawling    UMETA(DisplayName = "Crawling"),
    Climbing    UMETA(DisplayName = "Climbing"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Injured     UMETA(DisplayName = "Injured"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Fearful     UMETA(DisplayName = "Fearful")
};

// Movement quality modifiers based on survival state
UENUM(BlueprintType)
enum class EAnim_MovementQuality : uint8
{
    Normal      UMETA(DisplayName = "Normal"),
    Tired       UMETA(DisplayName = "Tired"),
    Hungry      UMETA(DisplayName = "Hungry"),
    Thirsty     UMETA(DisplayName = "Thirsty"),
    Cold        UMETA(DisplayName = "Cold"),
    Hot         UMETA(DisplayName = "Hot"),
    Sick        UMETA(DisplayName = "Sick"),
    Wounded     UMETA(DisplayName = "Wounded")
};

// Locomotion data for animation blending
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_LocomotionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    EAnim_SurvivalLocomotionState LocomotionState;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    EAnim_MovementQuality MovementQuality;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float StaminaPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float HealthPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float FearLevel;

    FAnim_LocomotionData()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsMoving = false;
        bIsInAir = false;
        LocomotionState = EAnim_SurvivalLocomotionState::Idle;
        MovementQuality = EAnim_MovementQuality::Normal;
        StaminaPercentage = 1.0f;
        HealthPercentage = 1.0f;
        FearLevel = 0.0f;
    }
};

/**
 * Survival Locomotion System - Handles realistic movement animation for prehistoric survival
 * Integrates with character survival stats to modify movement quality
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USurvivalLocomotionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    USurvivalLocomotionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Get current locomotion data for animation blueprint
    UFUNCTION(BlueprintCallable, Category = "Survival Locomotion")
    FAnim_LocomotionData GetLocomotionData() const { return CurrentLocomotionData; }

    // Update locomotion state based on character movement
    UFUNCTION(BlueprintCallable, Category = "Survival Locomotion")
    void UpdateLocomotionState(float DeltaTime);

    // Set movement quality based on survival stats
    UFUNCTION(BlueprintCallable, Category = "Survival Locomotion")
    void SetMovementQuality(float Health, float Stamina, float Hunger, float Thirst, float Temperature, float Fear);

    // Force specific locomotion state (for special situations)
    UFUNCTION(BlueprintCallable, Category = "Survival Locomotion")
    void ForceLocomotionState(EAnim_SurvivalLocomotionState NewState);

protected:
    // Current locomotion data
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    FAnim_LocomotionData CurrentLocomotionData;

    // Cached character reference
    UPROPERTY()
    ACharacter* OwnerCharacter;

    // Cached movement component
    UPROPERTY()
    UCharacterMovementComponent* MovementComponent;

    // Movement thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion Settings")
    float WalkThreshold = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion Settings")
    float RunThreshold = 300.0f;

    // Survival stat thresholds for movement quality
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Settings")
    float TiredStaminaThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Settings")
    float HungryThreshold = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Settings")
    float ThirstyThreshold = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Settings")
    float WoundedHealthThreshold = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Settings")
    float FearfulThreshold = 0.6f;

private:
    // Calculate movement direction relative to character facing
    float CalculateMovementDirection() const;

    // Determine locomotion state based on movement and survival stats
    EAnim_SurvivalLocomotionState DetermineLocomotionState() const;

    // Determine movement quality based on survival stats
    EAnim_MovementQuality DetermineMovementQuality() const;

    // Cached survival stats
    float CachedHealth = 1.0f;
    float CachedStamina = 1.0f;
    float CachedHunger = 1.0f;
    float CachedThirst = 1.0f;
    float CachedTemperature = 0.5f;
    float CachedFear = 0.0f;
};