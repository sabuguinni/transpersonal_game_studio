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

    UPROPERTY(BlueprintReadWrite, Category = "Anim|FootIK")
    FVector LeftFootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Anim|FootIK")
    FVector RightFootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Anim|FootIK")
    FRotator LeftFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "Anim|FootIK")
    FRotator RightFootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite, Category = "Anim|FootIK")
    float LeftFootAlpha = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Anim|FootIK")
    float RightFootAlpha = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Anim|FootIK")
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

    // --- Locomotion ---
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
    EAnim_LocomotionState Anim_LocomotionState;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
    float Anim_Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
    float Anim_Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
    bool Anim_bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
    bool Anim_bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
    bool Anim_bIsSprinting;

    // --- Foot IK ---
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK", meta = (AllowPrivateAccess = "true"))
    FAnim_FootIKData Anim_FootIK;

    // --- Aim Offset ---
    UPROPERTY(BlueprintReadOnly, Category = "Anim|AimOffset", meta = (AllowPrivateAccess = "true"))
    float Anim_AimPitch;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|AimOffset", meta = (AllowPrivateAccess = "true"))
    float Anim_AimYaw;

    // --- Survival Stats ---
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival", meta = (AllowPrivateAccess = "true"))
    float Anim_StaminaNormalized;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival", meta = (AllowPrivateAccess = "true"))
    float Anim_HealthNormalized;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival", meta = (AllowPrivateAccess = "true"))
    float Anim_FearNormalized;

    // --- Combat ---
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat", meta = (AllowPrivateAccess = "true"))
    EAnim_CombatStance Anim_CombatStance;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat", meta = (AllowPrivateAccess = "true"))
    bool Anim_bIsAttacking;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat", meta = (AllowPrivateAccess = "true"))
    bool Anim_bIsBlocking;

    // --- Thresholds ---
    UPROPERTY(EditDefaultsOnly, Category = "Anim|Config")
    float Anim_WalkSpeedThreshold = 10.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Anim|Config")
    float Anim_RunSpeedThreshold = 250.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Anim|Config")
    float Anim_SprintSpeedThreshold = 500.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Anim|Config")
    float Anim_FootIKTraceDistance = 50.0f;

private:
    void UpdateLocomotionState();
    void UpdateFootIK(float DeltaSeconds);
    void UpdateAimOffset();
    void UpdateSurvivalStats();

    UPROPERTY()
    TObjectPtr<class ACharacter> Anim_OwnerCharacter;
};
