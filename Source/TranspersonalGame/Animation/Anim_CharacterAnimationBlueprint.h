#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "SharedTypes.h"
#include "Anim_CharacterAnimationBlueprint.generated.h"

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
    float VelocityZ = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float GroundSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bShouldMove = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float MovementInputAmount = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsAccelerating = false;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_RotationData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Rotation")
    float YawOffset = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Rotation")
    float Lean = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Rotation")
    FRotator AimRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "Rotation")
    FRotator ActorRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "Rotation")
    float AimYaw = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Rotation")
    float AimPitch = 0.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_LayerBlendData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Layer Blending")
    float OverlayAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Layer Blending")
    float SpineAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Layer Blending")
    float HeadAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Layer Blending")
    float ArmAlpha_L = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Layer Blending")
    float ArmAlpha_R = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Layer Blending")
    float HandAlpha_L = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Layer Blending")
    float HandAlpha_R = 0.0f;
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API UAnim_CharacterAnimationBlueprint : public UAnimInstance
{
    GENERATED_BODY()

public:
    UAnim_CharacterAnimationBlueprint();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTime) override;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class ACharacter* OwningCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class UCharacterMovementComponent* OwningMovementComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    FAnim_MovementData MovementData;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    FAnim_RotationData RotationData;

    UPROPERTY(BlueprintReadOnly, Category = "Animation Data")
    FAnim_LayerBlendData LayerBlendData;

    UPROPERTY(BlueprintReadOnly, Category = "Animation States")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation States")
    bool bIsJumping = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation States")
    bool bIsFalling = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation States")
    bool bIsLanding = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation States")
    EAnim_MovementState CurrentMovementState = EAnim_MovementState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Animation States")
    EAnim_Gait CurrentGait = EAnim_Gait::Walking;

    UPROPERTY(BlueprintReadOnly, Category = "Animation States")
    EAnim_Stance CurrentStance = EAnim_Stance::Standing;

    UPROPERTY(BlueprintReadOnly, Category = "Animation States")
    EAnim_RotationMode CurrentRotationMode = EAnim_RotationMode::VelocityDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float WalkSpeed = 165.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float RunSpeed = 375.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float SprintSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float AnimatedWalkSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float AnimatedRunSpeed = 350.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float AnimatedSprintSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float GroundedLeanInterpSpeed = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float InAirLeanInterpSpeed = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float SmoothedAimingRotationInterpSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float InputYawOffsetInterpSpeed = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float TriggerPivotSpeedLimit = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float FootIKInterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float IKTraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float IKAdjustOffset = 2.0f;

protected:
    UFUNCTION(BlueprintCallable, Category = "Animation Updates")
    void UpdateMovementData(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation Updates")
    void UpdateRotationData(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation Updates")
    void UpdateInAirData(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation Updates")
    void UpdateLayerBlendData(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation Updates")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation States")
    void SetMovementState(EAnim_MovementState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation States")
    void SetGait(EAnim_Gait NewGait);

    UFUNCTION(BlueprintCallable, Category = "Animation States")
    void SetStance(EAnim_Stance NewStance);

    UFUNCTION(BlueprintCallable, Category = "Animation States")
    void SetRotationMode(EAnim_RotationMode NewRotationMode);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    float GetIKOffsetForSocket(FName SocketName) const;

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FRotator GetIKRotationForSocket(FName SocketName) const;

private:
    FVector PreviousVelocity = FVector::ZeroVector;
    float PreviousAimYaw = 0.0f;
    float SmoothedAimingAngle = 0.0f;
    float InputYawOffsetTime = 0.0f;
    float LastVelocityRotation = 0.0f;
    float LastMovementInputRotation = 0.0f;
    float TargetCharacterRotationDifference = 0.0f;
    float CharacterRotation = 0.0f;
    float TargetCharacterRotation = 0.0f;

    // Foot IK data
    float LeftFootIKOffset = 0.0f;
    float RightFootIKOffset = 0.0f;
    FRotator LeftFootIKRotation = FRotator::ZeroRotator;
    FRotator RightFootIKRotation = FRotator::ZeroRotator;
    float PelvisOffset = 0.0f;

    void CalculateGaitFromSpeed();
    void CalculateMovementDirection();
    void CalculateGroundedRotation(float DeltaTime);
    void CalculateInAirRotation(float DeltaTime);
    bool CanRotateInPlace() const;
    bool CanTurnInPlace() const;
    bool CanDynamicTransition() const;
    void SmoothCharacterRotation(FRotator Target, float TargetInterpSpeed, float ActorInterpSpeed, float DeltaTime);
    float CalculateDirection(const FVector& Velocity, const FRotator& BaseRotation) const;
    FVector CalculateRelativeAccelerationAmount() const;
    float GetAnimCurveValue(FName CurveName) const;
    void SetRootMotionMode(ERootMotionMode::Type InRootMotionMode);
    FRotator NormalizedDeltaRotator(FRotator A, FRotator B) const;
    float GetMappedSpeed() const;
    void InterpCharacterRotation(FRotator Current, FRotator Target, float InterpSpeed, float DeltaTime);
    void LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed, float DeltaTime);
    void SetMovementAction(EAnim_MovementAction NewAction);
    void EventOnPivot();
    void EventOnJumped();
    void EventOnLanded();
    bool IsMovementInput() const;
    bool HasMovementInput() const;
    bool HasVelocity() const;
    bool IsMoving() const;
    bool IsMovingOnGround() const;
    bool HasAnyRootMotion() const;
};