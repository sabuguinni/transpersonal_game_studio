#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoSurvivorAnimInstance.generated.h"

// ============================================================
// Locomotion state enum — drives state machine transitions
// ============================================================
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

// ============================================================
// Combat stance enum
// ============================================================
UENUM(BlueprintType)
enum class EAnim_CombatStance : uint8
{
    Unarmed     UMETA(DisplayName = "Unarmed"),
    Spear       UMETA(DisplayName = "Spear"),
    Bow         UMETA(DisplayName = "Bow"),
    Shield      UMETA(DisplayName = "Shield")
};

// ============================================================
// Foot IK data — bilateral foot placement on terrain
// ============================================================
USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    FVector LeftFootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    FVector RightFootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    FRotator LeftFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    FRotator RightFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    float LeftFootAlpha = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    float RightFootAlpha = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|FootIK")
    float PelvisOffset = 0.0f;
};

// ============================================================
// DinoSurvivorAnimInstance — main AnimInstance for the player
// ============================================================
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoSurvivorAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinoSurvivorAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ── Locomotion ──────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    EAnim_LocomotionState LocomotionState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    bool bIsSprinting;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    float LeanAngle;

    // ── Foot IK ─────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK",
              meta = (AllowPrivateAccess = "true"))
    FAnim_FootIKData FootIKData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
    float FootIKTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|FootIK")
    float FootIKInterpSpeed;

    // ── Aim Offset ──────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset",
              meta = (AllowPrivateAccess = "true"))
    float AimPitch;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset",
              meta = (AllowPrivateAccess = "true"))
    float AimYaw;

    // ── Survival Stats ──────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival",
              meta = (AllowPrivateAccess = "true"))
    float StaminaNormalized;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival",
              meta = (AllowPrivateAccess = "true"))
    float HealthNormalized;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival",
              meta = (AllowPrivateAccess = "true"))
    bool bIsExhausted;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival",
              meta = (AllowPrivateAccess = "true"))
    bool bIsInjured;

    // ── Combat ──────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat",
              meta = (AllowPrivateAccess = "true"))
    EAnim_CombatStance CombatStance;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat",
              meta = (AllowPrivateAccess = "true"))
    bool bIsAttacking;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat",
              meta = (AllowPrivateAccess = "true"))
    bool bIsBlocking;

    // ── Speed thresholds (configurable) ─────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Config")
    float WalkSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Config")
    float RunSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Config")
    float SprintSpeedThreshold;

protected:
    // Internal helpers
    void UpdateLocomotionState();
    void UpdateFootIK(float DeltaSeconds);
    void UpdateAimOffset();
    void UpdateSurvivalState();

    bool TraceFootToGround(const FName& FootSocketName, FVector& OutLocation, FRotator& OutRotation);

private:
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;
};
