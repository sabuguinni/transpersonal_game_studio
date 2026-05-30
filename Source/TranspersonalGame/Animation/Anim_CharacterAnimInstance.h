#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "SharedTypes.h"
#include "Anim_CharacterAnimInstance.generated.h"

USTRUCT(BlueprintType)
struct FAnim_MovementState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsAccelerating = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float LeanAmount = 0.0f;

    FAnim_MovementState()
    {
        Speed = 0.0f;
        Direction = 0.0f;
        bIsInAir = false;
        bIsCrouching = false;
        bIsAccelerating = false;
        LeanAmount = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FAnim_IKFootData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector FootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator FootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float IKAlpha = 0.0f;

    FAnim_IKFootData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
    }
};

/**
 * Animation Instance for character movement and IK foot placement
 * Handles idle, walk, run, jump states with smooth blending
 * Implements foot IK for terrain adaptation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_CharacterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTime) override;

    // Movement State
    UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
    FAnim_MovementState MovementState;

    // Character Reference
    UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    ACharacter* OwnerCharacter = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    UCharacterMovementComponent* MovementComponent = nullptr;

    // Animation Thresholds
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings", meta = (AllowPrivateAccess = "true"))
    float WalkSpeedThreshold = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings", meta = (AllowPrivateAccess = "true"))
    float RunSpeedThreshold = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Settings", meta = (AllowPrivateAccess = "true"))
    float DirectionDeadZone = 10.0f;

    // IK Settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK Settings", meta = (AllowPrivateAccess = "true"))
    bool bEnableFootIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK Settings", meta = (AllowPrivateAccess = "true"))
    float IKTraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK Settings", meta = (AllowPrivateAccess = "true"))
    float IKInterpSpeed = 15.0f;

    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    FAnim_IKFootData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    FAnim_IKFootData RightFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    float HipOffset = 0.0f;

private:
    void UpdateMovementValues(float DeltaTime);
    void UpdateFootIK(float DeltaTime);
    FAnim_IKFootData CalculateFootIK(const FName& SocketName, float DeltaTime);
    FVector GetIKFootLocation(const FVector& FootLocation);
    FRotator GetIKFootRotation(const FVector& ImpactLocation, const FVector& ImpactNormal);

    // Smoothing variables
    float LastSpeed = 0.0f;
    float LastDirection = 0.0f;
    float LastLeanAmount = 0.0f;
};