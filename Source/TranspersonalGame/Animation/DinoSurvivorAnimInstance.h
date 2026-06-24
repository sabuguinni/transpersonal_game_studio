#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoSurvivorAnimInstance.generated.h"

/**
 * Animation instance for the prehistoric survivor player character.
 * Drives the animation state machine: Idle → Walk → Run → Crouch → Jump → Combat.
 * Supports foot IK for uneven terrain adaptation.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoSurvivorAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinoSurvivorAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ── Locomotion State ──
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float GroundSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float GroundSpeedNormalized;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsFalling;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsSprinting;

    // ── Direction ──
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Direction")
    float MovementDirection;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Direction")
    float LeanAngle;

    // ── Survival State ──
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float StaminaNormalized;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsExhausted;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsInjured;

    // ── Combat State ──
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    bool bIsInCombat;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    bool bIsAiming;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    float AimPitch;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    float AimYaw;

    // ── Foot IK ──
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FVector LeftFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FVector RightFootIKLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FRotator LeftFootIKRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FRotator RightFootIKRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    float IKAlpha;

    // ── Speed thresholds ──
    UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
    float WalkSpeedThreshold;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
    float RunSpeedThreshold;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
    float SprintSpeedThreshold;

    UPROPERTY(EditDefaultsOnly, Category = "Animation|Config")
    float IKTraceDistance;

private:
    void UpdateLocomotion();
    void UpdateFootIK();
    void SolveFootIK(FName SocketName, FVector& OutLocation, FRotator& OutRotation);

    UPROPERTY()
    class ACharacter* OwnerCharacter;
};
