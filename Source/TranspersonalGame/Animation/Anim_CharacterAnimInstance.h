#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Anim_MotionMatchingComponent.h"
#include "SharedTypes.h"
#include "Anim_CharacterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float LeftFootIKOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float RightFootIKOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float PelvisIKOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FRotator LeftFootIKRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FRotator RightFootIKRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    bool bEnableFootIK;

    FAnim_IKData()
    {
        LeftFootIKOffset = 0.0f;
        RightFootIKOffset = 0.0f;
        PelvisIKOffset = 0.0f;
        LeftFootIKRotation = FRotator::ZeroRotator;
        RightFootIKRotation = FRotator::ZeroRotator;
        bEnableFootIK = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_CharacterAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimInstance();

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTime) override;

public:
    // Animation Blueprint accessible variables
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    float Direction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    float TurnRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    bool bIsInAir;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    bool bIsCrouching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    bool bIsAccelerating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    EAnim_LocomotionState LocomotionState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FAnim_IKData IKData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsInCombat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsAttacking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float HealthPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float StaminaPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float FearLevel;

    // Animation event functions
    UFUNCTION(BlueprintCallable, Category = "Animation Events")
    void OnJumpStart();

    UFUNCTION(BlueprintCallable, Category = "Animation Events")
    void OnJumpEnd();

    UFUNCTION(BlueprintCallable, Category = "Animation Events")
    void OnAttackStart();

    UFUNCTION(BlueprintCallable, Category = "Animation Events")
    void OnAttackEnd();

    UFUNCTION(BlueprintCallable, Category = "Animation Events")
    void OnFootstep(bool bIsLeftFoot);

    // IK functions
    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK();

    UFUNCTION(BlueprintCallable, Category = "IK")
    float GetFootIKOffset(bool bIsLeftFoot) const;

    UFUNCTION(BlueprintCallable, Category = "IK")
    FRotator GetFootIKRotation(bool bIsLeftFoot) const;

    UFUNCTION(BlueprintCallable, Category = "IK")
    float GetPelvisIKOffset() const;

protected:
    // Cached references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    class UAnim_MotionMatchingComponent* MotionMatchingComponent;

    // Animation state management
    void UpdateLocomotionState();
    void UpdateMovementValues();
    void UpdateCombatState();
    void UpdateSurvivalState();

    // IK calculation
    void CalculateFootIK();
    float TraceForFootIK(const FName& SocketName, FRotator& OutRotation);

    // Helper functions
    EAnim_LocomotionState DetermineLocomotionState() const;
    bool IsMovingOnGround() const;
    bool ShouldEnterIdleState() const;

private:
    // Previous frame data for interpolation
    float PreviousSpeed;
    float PreviousDirection;
    EAnim_LocomotionState PreviousLocomotionState;

    // IK settings
    UPROPERTY(EditAnywhere, Category = "IK Settings")
    float IKTraceDistance;

    UPROPERTY(EditAnywhere, Category = "IK Settings")
    float IKInterpSpeed;

    UPROPERTY(EditAnywhere, Category = "IK Settings")
    FName LeftFootSocketName;

    UPROPERTY(EditAnywhere, Category = "IK Settings")
    FName RightFootSocketName;

    // Animation smoothing
    UPROPERTY(EditAnywhere, Category = "Animation Settings")
    float SpeedSmoothingRate;

    UPROPERTY(EditAnywhere, Category = "Animation Settings")
    float DirectionSmoothingRate;

    UPROPERTY(EditAnywhere, Category = "Animation Settings")
    float TurnRateSmoothingRate;

    // Thresholds
    UPROPERTY(EditAnywhere, Category = "Animation Settings")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, Category = "Animation Settings")
    float RunSpeedThreshold;

    UPROPERTY(EditAnywhere, Category = "Animation Settings")
    float IdleSpeedThreshold;
};