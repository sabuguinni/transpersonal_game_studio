#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Anim_CharacterAnimController.generated.h"

/**
 * Main animation controller for tribal character movement and combat
 * Handles motion matching, IK foot placement, and survival animations
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_CharacterAnimController : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimController();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Movement State
    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsAccelerating;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching;

    // Combat State
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsHoldingWeapon;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    EAnim_WeaponType CurrentWeaponType;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsAttacking;

    // Survival State
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float HealthPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float StaminaPercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsInjured;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    EAnim_EmotionalState EmotionalState;

    // IK Foot Placement
    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float LeftFootIKOffset;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float RightFootIKOffset;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator LeftFootIKRotation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator RightFootIKRotation;

    // Motion Matching
    UPROPERTY(BlueprintReadOnly, Category = "MotionMatching")
    float MotionMatchingWeight;

    UPROPERTY(BlueprintReadOnly, Category = "MotionMatching")
    FVector DesiredVelocity;

    UPROPERTY(BlueprintReadOnly, Category = "MotionMatching")
    float TurnInPlaceAngle;

protected:
    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class ACharacter* OwningCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class UCharacterMovementComponent* CharacterMovement;

    // IK Functions
    UFUNCTION(BlueprintCallable, Category = "IK")
    void CalculateFootIK();

    UFUNCTION(BlueprintCallable, Category = "IK")
    float GetFootIKOffset(FName SocketName, float TraceDistance = 50.0f);

    UFUNCTION(BlueprintCallable, Category = "IK")
    FRotator GetFootIKRotation(FName SocketName);

    // Motion Matching Functions
    UFUNCTION(BlueprintCallable, Category = "MotionMatching")
    void UpdateMotionMatching();

    UFUNCTION(BlueprintCallable, Category = "MotionMatching")
    float CalculateMotionMatchingScore(const FVector& CurrentVelocity, const FVector& TargetVelocity);

    // Animation State Functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementState();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateCombatState();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateSurvivalState();

private:
    // IK Settings
    float IKTraceDistance;
    float IKInterpSpeed;
    
    // Motion Matching Settings
    float MotionMatchingThreshold;
    float VelocityMatchWeight;
    float DirectionMatchWeight;
    
    // Animation Smoothing
    float SpeedSmoothingRate;
    float DirectionSmoothingRate;
};