#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimInstance.generated.h"

/**
 * Animation Instance for the Transpersonal prehistoric survival character.
 * Drives locomotion blend space (idle/walk/run), jump states, and foot IK.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UTranspersonalAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ---- Locomotion ----

    /** Current movement speed (0=idle, 300=walk, 600=run) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float MovementSpeed;

    /** Lateral strafe direction (-1 left, 0 forward, 1 right) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    float StrafeDirection;

    /** True when character is in the air */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsInAir;

    /** True when character is crouching */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsCrouching;

    /** True when character is sprinting */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion")
    bool bIsSprinting;

    // ---- Survival State ----

    /** Stamina ratio 0-1 — affects animation speed and posture */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float StaminaRatio;

    /** Fear level 0-1 — affects trembling/alert pose blend */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float FearLevel;

    /** True when character is injured (limping blend) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsInjured;

    // ---- Foot IK ----

    /** Left foot IK target offset from ground trace */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FVector LeftFootIKOffset;

    /** Right foot IK target offset from ground trace */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    FVector RightFootIKOffset;

    /** Left foot IK alpha (0=disabled, 1=full IK) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    float LeftFootIKAlpha;

    /** Right foot IK alpha */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    float RightFootIKAlpha;

    /** Pelvis vertical adjustment to prevent foot IK stretching */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|FootIK")
    float PelvisOffset;

    // ---- Combat / Tool ----

    /** True when holding a weapon or tool */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    bool bIsArmed;

    /** Upper body aim pitch (-90 to 90) for aiming offset */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    float AimPitch;

    /** Upper body aim yaw for look-at blending */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    float AimYaw;

protected:
    /** Perform ground trace for foot IK at given socket location */
    void UpdateFootIK(float DeltaSeconds);

    /** Smooth interpolate foot IK offsets */
    void SmoothFootIK(FVector& CurrentOffset, const FVector& TargetOffset, float DeltaSeconds, float InterpSpeed = 15.0f);

    /** Cached owner character */
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    /** Cached movement component */
    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

private:
    /** Trace distance for foot IK ground detection */
    static constexpr float FootIKTraceDistance = 80.0f;

    /** Interpolation speed for foot IK smoothing */
    static constexpr float FootIKInterpSpeed = 20.0f;
};
