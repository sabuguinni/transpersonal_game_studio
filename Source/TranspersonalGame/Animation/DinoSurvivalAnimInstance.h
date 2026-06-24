#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoSurvivalAnimInstance.generated.h"

/**
 * Animation Instance for the prehistoric survivor player character.
 * Drives idle/walk/run/sprint/crouch/jump states based on character velocity and state.
 * Designed for use with UE5 Motion Matching and Foot IK.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoSurvivalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinoSurvivalAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ── Locomotion State ──────────────────────────────────────────────────────

    /** Current movement speed (cm/s) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float Speed;

    /** Lateral movement direction (-1 left, 0 forward, 1 right) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float Direction;

    /** True when character is in the air */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsInAir;

    /** True when character is crouching */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsCrouching;

    /** True when character is sprinting (speed > RunThreshold) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsSprinting;

    /** True when character is walking (speed > 0 but below RunThreshold) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsWalking;

    /** True when character is completely still */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsIdle;

    // ── Survival State ────────────────────────────────────────────────────────

    /** Fear level 0-1 — affects animation urgency and trembling */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float FearLevel;

    /** Stamina level 0-1 — affects movement animation weight */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float StaminaLevel;

    /** True when character is exhausted (stamina < 0.1) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    bool bIsExhausted;

    /** True when character is injured (health < 0.3) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    bool bIsInjured;

    // ── Foot IK ───────────────────────────────────────────────────────────────

    /** Left foot IK target location in world space */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    FVector LeftFootIKLocation;

    /** Right foot IK target location in world space */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    FVector RightFootIKLocation;

    /** Left foot IK alpha (0 = no IK, 1 = full IK) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    float LeftFootIKAlpha;

    /** Right foot IK alpha */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    float RightFootIKAlpha;

    /** Pelvis offset Z for foot IK ground adaptation */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    float PelvisOffset;

    // ── Thresholds ────────────────────────────────────────────────────────────

    /** Speed above which character is considered walking */
    UPROPERTY(EditDefaultsOnly, Category = "Anim|Config")
    float WalkThreshold;

    /** Speed above which character is considered running */
    UPROPERTY(EditDefaultsOnly, Category = "Anim|Config")
    float RunThreshold;

    /** Speed above which character is considered sprinting */
    UPROPERTY(EditDefaultsOnly, Category = "Anim|Config")
    float SprintThreshold;

    // ── Utility ───────────────────────────────────────────────────────────────

    /** Perform foot IK trace for given socket name, returns world hit location */
    UFUNCTION(BlueprintCallable, Category = "Anim|FootIK")
    FVector TraceFootIK(FName SocketName, float& OutAlpha);

private:
    /** Cached owner character */
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    /** Cached movement component */
    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    /** Update foot IK targets via line traces */
    void UpdateFootIK();

    /** Update locomotion state from velocity */
    void UpdateLocomotionState();

    /** Update survival state from character properties */
    void UpdateSurvivalState();
};
