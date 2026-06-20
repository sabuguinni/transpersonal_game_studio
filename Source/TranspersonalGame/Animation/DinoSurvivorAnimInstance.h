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

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoSurvivorAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinoSurvivorAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // --- Locomotion ---
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    EAnim_LocomotionState Anim_LocomotionState = EAnim_LocomotionState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    float Anim_Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    float Anim_Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    bool Anim_bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    bool Anim_bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    bool Anim_bIsSprinting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    float Anim_LeanAngle = 0.0f;

    // --- Foot IK ---
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK", meta = (AllowPrivateAccess = "true"))
    FAnim_FootIKData Anim_FootIK;

    // --- Aim Offset ---
    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset", meta = (AllowPrivateAccess = "true"))
    float Anim_AimPitch = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|AimOffset", meta = (AllowPrivateAccess = "true"))
    float Anim_AimYaw = 0.0f;

    // --- Survival State ---
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    float Anim_StaminaNormalized = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    float Anim_HealthNormalized = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    bool Anim_bIsExhausted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    bool Anim_bIsInjured = false;

    // --- Combat ---
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat", meta = (AllowPrivateAccess = "true"))
    EAnim_CombatStance Anim_CombatStance = EAnim_CombatStance::Unarmed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat", meta = (AllowPrivateAccess = "true"))
    bool Anim_bIsAttacking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat", meta = (AllowPrivateAccess = "true"))
    bool Anim_bIsBlocking = false;

    // --- Thresholds (tunable) ---
    UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
    float Anim_WalkSpeedThreshold = 10.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
    float Anim_RunSpeedThreshold = 250.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
    float Anim_SprintSpeedThreshold = 500.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
    float Anim_FootIKTraceDistance = 60.0f;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void Anim_TriggerAttack();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void Anim_SetCombatStance(EAnim_CombatStance NewStance);

private:
    void UpdateLocomotionState();
    void UpdateFootIK(float DeltaSeconds);
    void UpdateAimOffset();
    void UpdateSurvivalState();

    bool DoFootIKTrace(FName SocketName, FVector& OutLocation, FRotator& OutRotation, float& OutAlpha);

    UPROPERTY()
    class ACharacter* Anim_OwnerCharacter = nullptr;

    UPROPERTY()
    class UCharacterMovementComponent* Anim_MovementComponent = nullptr;
};
