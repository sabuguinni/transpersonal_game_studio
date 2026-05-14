#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Anim_CharacterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Swimming    UMETA(DisplayName = "Swimming")
};

UENUM(BlueprintType)
enum class EAnim_CombatState : uint8
{
    Peaceful    UMETA(DisplayName = "Peaceful"),
    Alert       UMETA(DisplayName = "Alert"),
    Combat      UMETA(DisplayName = "Combat"),
    Wounded     UMETA(DisplayName = "Wounded"),
    Dead        UMETA(DisplayName = "Dead")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MovementData
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
    float JumpHeight = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Acceleration = 0.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_SurvivalData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float HealthPercent = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float StaminaPercent = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float FearLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsInjured = false;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsExhausted = false;
};

/**
 * Animation Instance for prehistoric human characters
 * Handles movement states, survival animations, and combat responses
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_CharacterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimInstance();

protected:
    // Animation state variables
    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_MovementState MovementState = EAnim_MovementState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    EAnim_CombatState CombatState = EAnim_CombatState::Peaceful;

    UPROPERTY(BlueprintReadOnly, Category = "Movement Data")
    FAnim_MovementData MovementData;

    UPROPERTY(BlueprintReadOnly, Category = "Survival Data")
    FAnim_SurvivalData SurvivalData;

    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class ACharacter* OwnerCharacter = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class UCharacterMovementComponent* CharacterMovement = nullptr;

    // Animation parameters
    UPROPERTY(BlueprintReadOnly, Category = "Animation Parameters")
    float WalkSpeed = 150.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Parameters")
    float RunSpeed = 400.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Parameters")
    float CrouchSpeed = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Parameters")
    float TurnRate = 90.0f;

    // IK and procedural animation
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

    // Animation blending
    UPROPERTY(BlueprintReadOnly, Category = "Blending")
    float IdleToWalkBlend = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Blending")
    float WalkToRunBlend = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Blending")
    float CombatBlend = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Blending")
    float InjuryBlend = 0.0f;

public:
    // UAnimInstance interface
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Animation state functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementState();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateCombatState();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateSurvivalData();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateFootIK();

    // Animation triggers
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerJumpAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerCombatAnimation(bool bEnterCombat);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerInjuryAnimation(float InjurySeverity);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerFearAnimation(float FearIntensity);

    // Utility functions
    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsMoving() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsRunning() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsInCombat() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetMovementDirection() const;

protected:
    // Internal update functions
    void UpdateMovementData();
    void UpdateBlendWeights();
    void CalculateFootIK();
    
    // Helper functions
    float CalculateDirection(const FVector& Velocity, const FRotator& Rotation) const;
    EAnim_MovementState DetermineMovementState() const;
    EAnim_CombatState DetermineCombatState() const;
};