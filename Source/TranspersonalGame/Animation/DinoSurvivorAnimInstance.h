#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoSurvivorAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Swimming    UMETA(DisplayName = "Swimming"),
    Climbing    UMETA(DisplayName = "Climbing")
};

UENUM(BlueprintType)
enum class EAnim_CombatState : uint8
{
    Unarmed     UMETA(DisplayName = "Unarmed"),
    Spear       UMETA(DisplayName = "Spear"),
    Bow         UMETA(DisplayName = "Bow"),
    Club        UMETA(DisplayName = "Club"),
    Torch       UMETA(DisplayName = "Torch")
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
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    bool bIsSprinting = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    float LeanAmount = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    float AimPitch = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    float AimYaw = 0.0f;
};

USTRUCT(BlueprintType)
struct FAnim_IKData
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
    float PelvisOffset = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    bool bEnableFootIK = true;
};

/**
 * DinoSurvivorAnimInstance
 * Animation instance for the prehistoric human survivor character.
 * Drives locomotion blend spaces, foot IK, combat stances, and survival states.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoSurvivorAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinoSurvivorAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

    // ─── Locomotion State ───────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    EAnim_MovementState MovementState = EAnim_MovementState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    FAnim_LocomotionData LocomotionData;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    float GroundSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion", meta = (AllowPrivateAccess = "true"))
    bool bIsSprinting = false;

    // ─── Combat State ───────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat", meta = (AllowPrivateAccess = "true"))
    EAnim_CombatState CombatState = EAnim_CombatState::Unarmed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat", meta = (AllowPrivateAccess = "true"))
    bool bIsAttacking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat", meta = (AllowPrivateAccess = "true"))
    bool bIsBlocking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat", meta = (AllowPrivateAccess = "true"))
    float AimPitch = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat", meta = (AllowPrivateAccess = "true"))
    float AimYaw = 0.0f;

    // ─── Survival State ─────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    float StaminaNormalized = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    float HealthNormalized = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    bool bIsExhausted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    bool bIsInjured = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival", meta = (AllowPrivateAccess = "true"))
    float FearLevel = 0.0f;

    // ─── Foot IK ────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK", meta = (AllowPrivateAccess = "true"))
    FAnim_IKData IKData;

    // ─── Blueprint-callable helpers ─────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetCombatState(EAnim_CombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerAttackMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerDeathMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerHitReactMontage();

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldPlayExhaustionAnim() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetLeanAmount() const;

private:
    void UpdateLocomotionState();
    void UpdateCombatState();
    void UpdateSurvivalState();
    void UpdateFootIK(float DeltaSeconds);
    void SolveFootIK(const FName& FootSocketName, FVector& OutLocation, FRotator& OutRotation);

    UPROPERTY()
    TObjectPtr<class ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<class UCharacterMovementComponent> MovementComponent;

    // IK trace channel
    static constexpr float FootIKTraceDistance = 75.0f;
    static constexpr float PelvisAdjustSpeed = 15.0f;

    float PelvisOffsetTarget = 0.0f;
    float CurrentPelvisOffset = 0.0f;
};
