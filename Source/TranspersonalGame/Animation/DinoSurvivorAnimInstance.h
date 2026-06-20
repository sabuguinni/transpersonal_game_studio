#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoSurvivorAnimInstance.generated.h"

// ============================================================
// Enums — global scope, Anim_ prefix (RULE 1 + RULE 2)
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

UENUM(BlueprintType)
enum class EAnim_CombatStance : uint8
{
    Unarmed     UMETA(DisplayName = "Unarmed"),
    SpearReady  UMETA(DisplayName = "SpearReady"),
    BowReady    UMETA(DisplayName = "BowReady"),
    Defensive   UMETA(DisplayName = "Defensive")
};

// ============================================================
// Structs — global scope (RULE 1)
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
// Main AnimInstance class
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoSurvivorAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinoSurvivorAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ---- Locomotion ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    EAnim_LocomotionState Anim_LocomotionState = EAnim_LocomotionState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Anim_Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Anim_Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool Anim_bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool Anim_bIsSprinting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool Anim_bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool Anim_bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float Anim_FallSpeed = 0.0f;

    // ---- Aim Offset ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
    float Anim_AimPitch = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset")
    float Anim_AimYaw = 0.0f;

    // ---- Foot IK ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FAnim_FootIKData Anim_FootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    bool Anim_bFootIKEnabled = true;

    // ---- Combat ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    EAnim_CombatStance Anim_CombatStance = EAnim_CombatStance::Unarmed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    bool Anim_bIsAttacking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    bool Anim_bIsBlocking = false;

    // ---- Survival State ----
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float Anim_StaminaNormalized = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float Anim_HealthNormalized = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool Anim_bIsExhausted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool Anim_bIsInjured = false;

protected:
    // Internal helpers
    void UpdateLocomotionState();
    void UpdateFootIK(float DeltaSeconds);
    void UpdateAimOffset();
    void UpdateSurvivalState();

    // Cached owner pawn
    UPROPERTY()
    class ACharacter* Anim_OwnerCharacter = nullptr;

    UPROPERTY()
    class UCharacterMovementComponent* Anim_MovementComponent = nullptr;

    // Foot IK trace helpers
    bool TraceFootIK(const FName& BoneName, FVector& OutLocation, FRotator& OutRotation, float& OutAlpha);
    float Anim_FootIKTraceDistance = 50.0f;
};
