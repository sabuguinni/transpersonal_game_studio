#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PrimitiveAnimationController.h"
#include "../SharedTypes.h"
#include "PrehistoricAnimInstance.generated.h"

USTRUCT(BlueprintType)
struct FAnim_BlendSpaceInput
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float Lean = 0.0f;

    FAnim_BlendSpaceInput()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        Lean = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FAnim_IKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector LeftFootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector RightFootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator LeftFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator RightFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float LeftFootIKAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float RightFootIKAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float HipOffset = 0.0f;

    FAnim_IKData()
    {
        LeftFootLocation = FVector::ZeroVector;
        RightFootLocation = FVector::ZeroVector;
        LeftFootRotation = FRotator::ZeroRotator;
        RightFootRotation = FRotator::ZeroRotator;
        LeftFootIKAlpha = 0.0f;
        RightFootIKAlpha = 0.0f;
        HipOffset = 0.0f;
    }
};

/**
 * Prehistoric Animation Instance for primitive human characters
 * Handles locomotion, foot IK, and survival action animations
 * Designed for realistic prehistoric human movement and behavior
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPrehistoricAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPrehistoricAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

public:
    // Core animation variables (exposed to Blueprint)
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    bool bIsRunning = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    bool bIsJumping = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    bool bIsFalling = false;

    // Blend space inputs
    UPROPERTY(BlueprintReadOnly, Category = "Animation|BlendSpace")
    FAnim_BlendSpaceInput BlendSpaceInput;

    // Animation states
    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    EAnim_MovementState MovementState = EAnim_MovementState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    EAnim_ActionState ActionState = EAnim_ActionState::None;

    // Foot IK data
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FAnim_IKData FootIKData;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    bool bEnableFootIK = true;

    // Survival action flags
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Actions")
    bool bIsGathering = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Actions")
    bool bIsCrafting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Actions")
    bool bIsInCombat = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Actions")
    bool bIsInteracting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Actions")
    bool bIsEating = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Actions")
    bool bIsDrinking = false;

    // Animation assets (to be set in Blueprint)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Assets")
    UBlendSpace* LocomotionBlendSpace = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Assets")
    UAnimSequence* IdleAnimation = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Assets")
    UAnimSequence* JumpStartAnimation = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Assets")
    UAnimSequence* JumpLoopAnimation = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Assets")
    UAnimSequence* JumpEndAnimation = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Assets")
    UAnimMontage* GatherMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Assets")
    UAnimMontage* CraftMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Assets")
    UAnimMontage* CombatMontage = nullptr;

    // IK configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|IK Config")
    float FootIKTraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|IK Config")
    float FootIKInterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|IK Config")
    FName LeftFootBoneName = TEXT("foot_l");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|IK Config")
    FName RightFootBoneName = TEXT("foot_r");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|IK Config")
    FName HipBoneName = TEXT("pelvis");

    // Animation thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Config")
    float MovingSpeedThreshold = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Config")
    float RunningSpeedThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Config")
    float DirectionChangeThreshold = 90.0f;

protected:
    // Component references
    UPROPERTY(BlueprintReadOnly, Category = "References")
    ACharacter* OwnerCharacter = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    UCharacterMovementComponent* MovementComponent = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    UPrimitiveAnimationController* AnimController = nullptr;

    // Internal state
    FVector LastVelocity = FVector::ZeroVector;
    float LastDirection = 0.0f;
    float DirectionChangeTimer = 0.0f;

private:
    void UpdateMovementVariables();
    void UpdateActionVariables();
    void UpdateFootIK(float DeltaTime);
    void CalculateFootIKData(const FName& FootBoneName, FVector& OutLocation, FRotator& OutRotation, float& OutIKAlpha);
    FVector GetFootWorldLocation(const FName& FootBoneName) const;
    bool TraceForGround(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal) const;
};