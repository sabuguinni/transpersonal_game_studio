#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "../Core/SharedTypes.h"
#include "Anim_CharacterAnimInstance.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MovementState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsAccelerating = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsRunning = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float AimPitch = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float AimYaw = 0.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_WeaponState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Weapon")
    bool bIsHoldingWeapon = false;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon")
    EWeaponType WeaponType = EWeaponType::None;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon")
    bool bIsAttacking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon")
    bool bIsBlocking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon")
    float AttackComboIndex = 0.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_SurvivalState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float HealthPercentage = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float StaminaPercentage = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float FearLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsInjured = false;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsExhausted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsTerrorized = false;
};

/**
 * Animation Instance for primitive human characters
 * Handles movement, weapon, and survival state animations
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_CharacterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTime) override;

protected:
    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class UCharacterMovementComponent* MovementComponent;

    // Animation states
    UPROPERTY(BlueprintReadOnly, Category = "Animation States")
    FAnim_MovementState MovementState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation States")
    FAnim_WeaponState WeaponState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation States")
    FAnim_SurvivalState SurvivalState;

    // Animation parameters
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float WalkSpeed = 150.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float RunSpeed = 400.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float JumpThreshold = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float AccelerationThreshold = 10.0f;

    // Blend space parameters
    UPROPERTY(BlueprintReadOnly, Category = "Blend Space")
    float BlendSpaceX = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Blend Space")
    float BlendSpaceY = 0.0f;

    // IK parameters
    UPROPERTY(BlueprintReadOnly, Category = "IK")
    bool bEnableFootIK = true;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float LeftFootIKOffset = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float RightFootIKOffset = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator LeftFootIKRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator RightFootIKRotation = FRotator::ZeroRotator;

private:
    // Update functions
    void UpdateMovementState(float DeltaTime);
    void UpdateWeaponState(float DeltaTime);
    void UpdateSurvivalState(float DeltaTime);
    void UpdateBlendSpaceParameters(float DeltaTime);
    void UpdateFootIK(float DeltaTime);

    // Helper functions
    float CalculateDirection(const FVector& Velocity, const FRotator& Rotation) const;
    bool IsMovingOnGround() const;
    FVector GetFootLocation(const FName& SocketName) const;
    float PerformFootTrace(const FVector& FootLocation, FRotator& OutRotation) const;
};