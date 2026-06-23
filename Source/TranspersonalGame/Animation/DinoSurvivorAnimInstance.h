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
    Fall        UMETA(DisplayName = "Fall"),
    Land        UMETA(DisplayName = "Land"),
    Climb       UMETA(DisplayName = "Climb"),
    Swim        UMETA(DisplayName = "Swim"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EAnim_CombatStance : uint8
{
    Unarmed     UMETA(DisplayName = "Unarmed"),
    Spear       UMETA(DisplayName = "Spear"),
    Club        UMETA(DisplayName = "Club"),
    Bow         UMETA(DisplayName = "Bow"),
    Shield      UMETA(DisplayName = "Shield"),
    Torch       UMETA(DisplayName = "Torch")
};

UENUM(BlueprintType)
enum class EAnim_SurvivalCondition : uint8
{
    Healthy     UMETA(DisplayName = "Healthy"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Wounded     UMETA(DisplayName = "Wounded"),
    Starving    UMETA(DisplayName = "Starving"),
    Freezing    UMETA(DisplayName = "Freezing"),
    Terrified   UMETA(DisplayName = "Terrified")
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
    float Pitch = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    bool bIsAccelerating = false;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    float LeanAngle = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|Locomotion")
    float VerticalVelocity = 0.0f;
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
    float LeftFootAlpha = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    float RightFootAlpha = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
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
    virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

    // ── Locomotion ──────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    FAnim_LocomotionData LocomotionData;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    EAnim_LocomotionState LocomotionState;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float GroundSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float MovementDirection = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float JumpVelocity = 0.0f;

    // ── Combat ───────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    EAnim_CombatStance CombatStance;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    bool bIsAttacking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    bool bIsBlocking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    float AimPitch = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    float AimYaw = 0.0f;

    // ── Survival Condition ───────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    EAnim_SurvivalCondition SurvivalCondition;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float ExhaustionAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float WoundAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float FearAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float StarvationAlpha = 0.0f;

    // ── IK ───────────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FAnim_IKData IKData;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    bool bEnableFootIK = true;

    // ── Blend Weights ────────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Blend")
    float WalkRunAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Blend")
    float UpperBodyLayerWeight = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Blend")
    float AdditiveBreathingWeight = 1.0f;

    // ── Blueprint-callable helpers ───────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetCombatStance(EAnim_CombatStance NewStance);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerAttackMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerDeathMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateSurvivalCondition(float Health, float Stamina, float Hunger, float Fear);

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool ShouldPlayLimpAnimation() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetLocomotionBlendAlpha() const;

protected:
    void UpdateLocomotionState(float DeltaSeconds);
    void UpdateIKFootPlacement(float DeltaSeconds);
    void UpdateSurvivalBlends(float DeltaSeconds);
    void UpdateAimOffsets(float DeltaSeconds);
    void SolveFootIK(bool bIsLeftFoot, FVector& OutLocation, FRotator& OutRotation, float& OutAlpha);

private:
    UPROPERTY()
    TObjectPtr<class ACharacter> OwnerCharacter;

    UPROPERTY()
    TObjectPtr<class UCharacterMovementComponent> MovementComponent;

    float TimeSinceLastFootstep = 0.0f;
    float PreviousSpeed = 0.0f;
    float LandingRecoveryTimer = 0.0f;
    bool bWasInAir = false;
};
