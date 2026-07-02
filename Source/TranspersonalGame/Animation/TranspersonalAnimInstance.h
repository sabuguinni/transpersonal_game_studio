#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Run         UMETA(DisplayName = "Run"),
    Sprint      UMETA(DisplayName = "Sprint"),
    Jump        UMETA(DisplayName = "Jump"),
    Fall        UMETA(DisplayName = "Fall"),
    Land        UMETA(DisplayName = "Land"),
    Crouch      UMETA(DisplayName = "Crouch"),
    Attack      UMETA(DisplayName = "Attack"),
    Death       UMETA(DisplayName = "Death")
};

UENUM(BlueprintType)
enum class EAnim_StanceType : uint8
{
    Upright     UMETA(DisplayName = "Upright"),
    Crouched    UMETA(DisplayName = "Crouched"),
    Prone       UMETA(DisplayName = "Prone")
};

USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    FVector LeftFootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    FVector RightFootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    FRotator LeftFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    FRotator RightFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    float LeftFootAlpha = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    float RightFootAlpha = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    float PelvisOffset = 0.0f;
};

USTRUCT(BlueprintType)
struct FAnim_LocomotionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    bool bIsSprinting = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    float LeanAmount = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    float AimPitch = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    float AimYaw = 0.0f;
};

/**
 * UTranspersonalAnimInstance
 * Main Animation Instance for the player character and humanoid NPCs.
 * Drives locomotion blend space, foot IK, aim offset, and state transitions.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UTranspersonalAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ── Locomotion ──────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    FAnim_LocomotionData LocomotionData;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    EAnim_LocomotionState LocomotionState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    EAnim_StanceType StanceType;

    // ── Foot IK ─────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FAnim_FootIKData FootIKData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|IK")
    bool bEnableFootIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|IK")
    float FootIKTraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|IK")
    float FootIKInterpSpeed = 15.0f;

    // ── Aim Offset ──────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Aim")
    float AimPitch = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Aim")
    float AimYaw = 0.0f;

    // ── State Flags ─────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    bool bIsAlive = true;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    bool bIsAttacking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
    float AttackPlayRate = 1.0f;

    // ── Blend Weights ───────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Blend")
    float UpperBodyLayerWeight = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Blend")
    float LowerBodyLayerWeight = 1.0f;

    // ── Blueprint Events ────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerAttackMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerDeathMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetAttackState(bool bAttacking);

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetMovementSpeed() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetMovementDirection() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool GetIsMoving() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool GetIsInAir() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_LocomotionState GetLocomotionState() const;

private:
    void UpdateLocomotionData(float DeltaSeconds);
    void UpdateLocomotionState();
    void UpdateFootIK(float DeltaSeconds);
    void UpdateAimOffset(float DeltaSeconds);
    void UpdateBlendWeights(float DeltaSeconds);

    bool PerformFootTrace(FName SocketName, FVector& OutLocation, FRotator& OutRotation);
    float CalculateDirection(const FVector& Velocity, const FRotator& ActorRotation) const;

    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    float LandingTimer = 0.0f;
    float SprintSpeedThreshold = 500.0f;
    float RunSpeedThreshold = 200.0f;
    float WalkSpeedThreshold = 10.0f;
};
