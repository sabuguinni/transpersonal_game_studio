#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PrimitiveAnimationController.h"
#include "SharedTypes.h"
#include "PrehistoricAnimInstance.generated.h"

/**
 * Prehistoric Animation Instance
 * Custom AnimInstance for prehistoric human characters
 * Handles animation logic for survival gameplay without spiritual elements
 */
UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UPrehistoricAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPrehistoricAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
    // === ANIMATION VARIABLES (Blueprint accessible) ===

    /** Current movement speed (horizontal) */
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float Speed = 0.0f;

    /** Current movement direction relative to character forward */
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float Direction = 0.0f;

    /** Vertical velocity (for jump/fall detection) */
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    float VerticalVelocity = 0.0f;

    /** Is the character in the air? */
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsInAir = false;

    /** Is the character crouching? */
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsCrouching = false;

    /** Is the character moving? */
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsMoving = false;

    /** Is the character running (vs walking)? */
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    bool bIsRunning = false;

    /** Current animation state from controller */
    UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    EAnim_MovementState CurrentAnimationState = EAnim_MovementState::Idle;

    /** Time since last state change (for transition blending) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    float TimeSinceStateChange = 0.0f;

    // === SURVIVAL ANIMATION VARIABLES ===

    /** Character health percentage (affects posture) */
    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    float HealthPercentage = 1.0f;

    /** Character stamina percentage (affects movement style) */
    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    float StaminaPercentage = 1.0f;

    /** Character fear level (affects movement urgency) */
    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    float FearLevel = 0.0f;

    /** Is character carrying something heavy? */
    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    bool bIsCarryingLoad = false;

    /** Character temperature (affects idle animations) */
    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    float Temperature = 0.5f; // 0 = freezing, 1 = hot

    // === ANIMATION SETTINGS ===

    /** Speed threshold for walking animation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float WalkSpeedThreshold = 150.0f;

    /** Speed threshold for running animation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float RunSpeedThreshold = 400.0f;

    /** Minimum vertical velocity to trigger falling animation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float FallingVelocityThreshold = -200.0f;

    /** How quickly to update direction changes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float DirectionSmoothingRate = 10.0f;

    /** How quickly to update speed changes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float SpeedSmoothingRate = 5.0f;

    // === DEBUGGING ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDebugAnimationValues = false;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogAnimationState();

private:
    // === CACHED REFERENCES ===

    UPROPERTY()
    TObjectPtr<ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<UCharacterMovementComponent> MovementComponent;

    UPROPERTY()
    TObjectPtr<UPrimitiveAnimationController> AnimationController;

    // === INTERNAL STATE ===

    FVector LastFrameVelocity = FVector::ZeroVector;
    EAnim_MovementState LastAnimationState = EAnim_MovementState::Idle;
    float StateChangeTimer = 0.0f;

    // Smoothed values for animation
    float SmoothedSpeed = 0.0f;
    float SmoothedDirection = 0.0f;

    // === INTERNAL METHODS ===

    /**
     * Update movement-related animation variables
     */
    void UpdateMovementVariables(float DeltaSeconds);

    /**
     * Update survival-related animation variables
     */
    void UpdateSurvivalVariables();

    /**
     * Calculate movement direction relative to character facing
     */
    float CalculateDirection(const FVector& Velocity, const FRotator& Rotation);

    /**
     * Smooth a value towards a target
     */
    float SmoothValue(float Current, float Target, float Rate, float DeltaTime);

    /**
     * Get survival stats from character (if available)
     */
    void UpdateSurvivalStats();
};