#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoSurvivorAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Run         UMETA(DisplayName = "Run"),
    Sprint      UMETA(DisplayName = "Sprint"),
    Crouch      UMETA(DisplayName = "Crouch"),
    CrouchWalk  UMETA(DisplayName = "CrouchWalk"),
    Jump        UMETA(DisplayName = "Jump"),
    Fall        UMETA(DisplayName = "Fall")
};

UENUM(BlueprintType)
enum class EAnim_CombatStance : uint8
{
    Unarmed     UMETA(DisplayName = "Unarmed"),
    Spear       UMETA(DisplayName = "Spear"),
    Bow         UMETA(DisplayName = "Bow"),
    Shield      UMETA(DisplayName = "Shield")
};

USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "FootIK")
    FVector LeftFootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "FootIK")
    FVector RightFootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "FootIK")
    FRotator LeftFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "FootIK")
    FRotator RightFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "FootIK")
    float LeftFootAlpha = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "FootIK")
    float RightFootAlpha = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "FootIK")
    float PelvisOffset = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoSurvivorAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinoSurvivorAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ── Locomotion ──────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    float LeanAngle = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsSprinting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
    EAnim_LocomotionState LocomotionState = EAnim_LocomotionState::Idle;

    // ── Foot IK ─────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "FootIK")
    FAnim_FootIKData FootIKData;

    // ── Aim Offset ──────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "AimOffset")
    float AimPitch = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "AimOffset")
    float AimYaw = 0.0f;

    // ── Survival State ──────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float StaminaNormalized = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    float FearNormalized = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsExhausted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Survival")
    bool bIsInjured = false;

    // ── Combat ──────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    EAnim_CombatStance CombatStance = EAnim_CombatStance::Unarmed;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsAttacking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIsBlocking = false;

    // ── Foot IK helpers ─────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "FootIK")
    void UpdateFootIK(float DeltaSeconds);

    UFUNCTION(BlueprintCallable, Category = "FootIK")
    FVector GetFootIKLocation(FName SocketName, float TraceDistance = 80.0f);

private:
    UPROPERTY()
    class ACharacter* OwnerCharacter = nullptr;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent = nullptr;

    float LeanAngleTarget = 0.0f;
    float LeanAngleSmoothed = 0.0f;
};
