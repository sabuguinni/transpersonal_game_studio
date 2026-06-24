#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoSurvivalAnimInstance.generated.h"

/**
 * Animation Instance for the player character in the prehistoric survival game.
 * Drives idle/walk/run/jump blend states based on movement velocity and physics state.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoSurvivalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinoSurvivalAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ── Movement State ──────────────────────────────────────────────────────

    /** Current ground speed (cm/s) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    float GroundSpeed;

    /** True when the character has velocity > 10 cm/s */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    bool bIsMoving;

    /** True when character is in the air */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    bool bIsInAir;

    /** True when character is sprinting (speed > RunThreshold) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    bool bIsSprinting;

    /** True when character is crouching */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    bool bIsCrouching;

    /** Direction of movement relative to character facing (-180 to 180) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    float MovementDirection;

    /** Vertical velocity — used to drive jump/fall blend */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    float VerticalVelocity;

    // ── Survival State ───────────────────────────────────────────────────────

    /** 0-100 health — drives injury/limp animations */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float Health;

    /** 0-100 stamina — drives exhaustion blend */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    float Stamina;

    /** True when health < 30 — triggers limp additive layer */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsInjured;

    /** True when stamina < 20 — triggers exhaustion pose */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Survival")
    bool bIsExhausted;

    // ── IK ───────────────────────────────────────────────────────────────────

    /** Left foot IK target world location */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FVector LeftFootIKLocation;

    /** Right foot IK target world location */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    FVector RightFootIKLocation;

    /** Pelvis offset to keep character grounded on slopes */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
    float PelvisOffset;

    // ── Thresholds ───────────────────────────────────────────────────────────

    /** Speed above which character is considered sprinting */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation|Config")
    float RunThreshold;

    /** Speed below which idle animation plays */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation|Config")
    float IdleThreshold;

protected:
    /** Cached reference to owning character */
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    /** Cached movement component */
    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    /** Update foot IK positions via line traces */
    void UpdateFootIK();

    /** Compute pelvis offset from foot IK delta */
    void UpdatePelvisOffset();
};
