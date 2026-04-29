#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "PrimitiveAnimationController.h"
#include "../SharedTypes.h"
#include "PrimitiveAnimInstance.generated.h"

/**
 * Animation Blueprint instance for primitive characters
 * Handles animation logic, state machines, and blend spaces
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPrimitiveAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPrimitiveAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
    // Animation state variables (exposed to Blueprint)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation State")
    EAnim_MovementState MovementState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation State")
    EAnim_CombatState CombatState;

    // Movement parameters for blend spaces
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    float Speed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    float Direction;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    float GroundDistance;

    // Combat parameters
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    bool bIsAttacking;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    bool bIsBlocking;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    bool bHasWeapon;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    float AttackSpeed;

    // Foot IK parameters
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
    float LeftFootIKOffset;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
    float RightFootIKOffset;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
    FRotator LeftFootIKRotation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
    FRotator RightFootIKRotation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
    bool bEnableFootIK;

    // Animation assets (can be set in Blueprint)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UBlendSpace> LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UBlendSpace1D> IdleBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimSequence> JumpStartAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimSequence> JumpLoopAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimSequence> JumpEndAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimSequence> CrouchIdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimSequence> CrouchWalkAnimation;

    // Foot IK settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    float FootIKTraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    float FootIKInterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    FName LeftFootBoneName = TEXT("foot_l");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    FName RightFootBoneName = TEXT("foot_r");

protected:
    // Animation update functions
    void UpdateMovementParameters();
    void UpdateCombatParameters();
    void UpdateFootIK();

    // Foot IK helper functions
    float CalculateFootIKOffset(const FName& FootBoneName, float& OutFootRotation);
    FVector GetFootWorldLocation(const FName& FootBoneName) const;

    // Component references
    UPROPERTY()
    TObjectPtr<UPrimitiveAnimationController> AnimationController;

    UPROPERTY()
    TObjectPtr<class ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<class UCharacterMovementComponent> MovementComponent;

    // Cached values for smooth interpolation
    float CachedSpeed;
    float CachedDirection;
    float PreviousLeftFootOffset;
    float PreviousRightFootOffset;
    FRotator PreviousLeftFootRotation;
    FRotator PreviousRightFootRotation;

private:
    // Internal timing
    float LastUpdateTime;
    float DeltaTimeAccumulator;
};