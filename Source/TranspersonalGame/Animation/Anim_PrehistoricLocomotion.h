#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimSequence.h"
#include "Kismet/KismetMathLibrary.h"
#include "Anim_PrehistoricLocomotion.generated.h"

// Forward declarations
class ATranspersonalCharacter;
class UCharacterMovementComponent;

/**
 * Animation states for prehistoric survival gameplay
 */
UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Climbing    UMETA(DisplayName = "Climbing"),
    Swimming    UMETA(DisplayName = "Swimming")
};

/**
 * Survival action states for prehistoric character
 */
UENUM(BlueprintType)
enum class EAnim_SurvivalAction : uint8
{
    None            UMETA(DisplayName = "None"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Building        UMETA(DisplayName = "Building"),
    Cooking         UMETA(DisplayName = "Cooking"),
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Injured         UMETA(DisplayName = "Injured"),
    Exhausted       UMETA(DisplayName = "Exhausted")
};

/**
 * Prehistoric character animation instance with survival-focused locomotion
 * Handles realistic movement animations for a paleontologist surviving in Cretaceous period
 */
UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UAnim_PrehistoricLocomotion : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_PrehistoricLocomotion();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    ATranspersonalCharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    UCharacterMovementComponent* MovementComponent;

    // === LOCOMOTION VARIABLES ===
    
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion", meta = (AllowPrivateAccess = "true"))
    EAnim_LocomotionState CurrentLocomotionState;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion", meta = (AllowPrivateAccess = "true"))
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion", meta = (AllowPrivateAccess = "true"))
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion", meta = (AllowPrivateAccess = "true"))
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion", meta = (AllowPrivateAccess = "true"))
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion", meta = (AllowPrivateAccess = "true"))
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion", meta = (AllowPrivateAccess = "true"))
    float VelocityZ;

    // === SURVIVAL VARIABLES ===
    
    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    EAnim_SurvivalAction CurrentSurvivalAction;

    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    float HealthPercent;

    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    float StaminaPercent;

    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    float FearLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    bool bIsExhausted;

    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    bool bIsInjured;

    // === ANIMATION ASSETS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UBlendSpace* LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimSequence* IdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* LandingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* GatheringMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* CraftingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    UAnimMontage* InjuredMontage;

    // === CONFIGURATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Config")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Config")
    float RunSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Config")
    float DirectionSmoothSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Config")
    float SpeedSmoothSpeed;

public:
    // === PUBLIC INTERFACE ===
    
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetSurvivalAction(EAnim_SurvivalAction NewAction);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlaySurvivalMontage(EAnim_SurvivalAction Action);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateSurvivalStats(float Health, float Stamina, float Fear);

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldPlayInjuredAnimation() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldPlayExhaustedAnimation() const;

private:
    // Internal update functions
    void UpdateLocomotionState();
    void UpdateMovementVariables();
    void UpdateSurvivalVariables();
    void SmoothValues(float DeltaTime);

    // Cached values for smoothing
    float CachedSpeed;
    float CachedDirection;
    float LastUpdateTime;
};