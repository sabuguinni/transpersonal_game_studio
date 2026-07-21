#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Kismet/KismetMathLibrary.h"
#include "TranspersonalGame/TranspersonalGame.h"
#include "Anim_PrimitiveCharacterInstance.generated.h"

class ATranspersonalCharacter;

/**
 * Animation Instance for primitive prehistoric characters
 * Handles basic locomotion, survival animations, and terrain adaptation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_PrimitiveCharacterInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_PrimitiveCharacterInstance();

protected:
    // === CORE ANIMATION VARIABLES ===
    
    /** Current movement speed */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion", meta = (AllowPrivateAccess = "true"))
    float Speed;
    
    /** Is character in air (jumping/falling) */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion", meta = (AllowPrivateAccess = "true"))
    bool bIsInAir;
    
    /** Is character moving */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion", meta = (AllowPrivateAccess = "true"))
    bool bIsMoving;
    
    /** Movement direction relative to character rotation */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion", meta = (AllowPrivateAccess = "true"))
    float Direction;
    
    /** Character lean angle for turns */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion", meta = (AllowPrivateAccess = "true"))
    float LeanAngle;
    
    // === SURVIVAL ANIMATION STATES ===
    
    /** Is character crouching (stealth mode) */
    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    bool bIsCrouching;
    
    /** Is character exhausted (low stamina) */
    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    bool bIsExhausted;
    
    /** Is character injured (affects movement) */
    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    bool bIsInjured;
    
    /** Fear level affects posture and movement */
    UPROPERTY(BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    float FearLevel;
    
    // === IK FOOT PLACEMENT ===
    
    /** Enable IK foot placement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    bool bEnableFootIK;
    
    /** Left foot IK offset */
    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    FVector LeftFootOffset;
    
    /** Right foot IK offset */
    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    FVector RightFootOffset;
    
    /** Pelvis offset for IK */
    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    FVector PelvisOffset;
    
    /** Left foot rotation for IK */
    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    FRotator LeftFootRotation;
    
    /** Right foot rotation for IK */
    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    FRotator RightFootRotation;
    
    // === IK CONFIGURATION ===
    
    /** Distance to trace for foot placement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float FootTraceDistance;
    
    /** Interpolation speed for foot offsets */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float FootOffsetInterpSpeed;
    
    /** Interpolation speed for pelvis offset */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float PelvisOffsetInterpSpeed;
    
    // === ANIMATION MONTAGES ===
    
    /** Idle animation montage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    UAnimMontage* IdleMontage;
    
    /** Jump animation montage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    UAnimMontage* JumpMontage;
    
    /** Landing animation montage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    UAnimMontage* LandMontage;
    
    /** Crouch animation montage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    UAnimMontage* CrouchMontage;
    
    // === BLEND SPACES ===
    
    /** Locomotion blend space */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
    UBlendSpace* LocomotionBlendSpace;

public:
    // === CORE OVERRIDES ===
    
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTime) override;

protected:
    // === ANIMATION UPDATE FUNCTIONS ===
    
    /** Update basic locomotion variables */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateLocomotionVariables();
    
    /** Update survival state variables */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateSurvivalStates();
    
    /** Update IK foot placement */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateFootIK(float DeltaTime);
    
    /** Perform foot trace for IK */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    FVector PerformFootTrace(FName SocketName, float TraceDistance);
    
    /** Calculate foot rotation for surface normal */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    FRotator CalculateFootRotation(FVector SurfaceNormal);

private:
    // === CACHED REFERENCES ===
    
    /** Cached reference to owning character */
    UPROPERTY()
    ATranspersonalCharacter* OwningCharacter;
    
    /** Cached reference to character movement component */
    UPROPERTY()
    UCharacterMovementComponent* MovementComponent;
    
    // === INTERNAL STATE ===
    
    /** Previous frame velocity for direction calculation */
    FVector PreviousVelocity;
    
    /** Target foot offsets for interpolation */
    FVector TargetLeftFootOffset;
    FVector TargetRightFootOffset;
    FVector TargetPelvisOffset;
    
    /** Target foot rotations for interpolation */
    FRotator TargetLeftFootRotation;
    FRotator TargetRightFootRotation;
};