#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

// Movement state enum for the prehistoric human player
UENUM(BlueprintType)
enum class EAnim_PlayerMovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Climbing    UMETA(DisplayName = "Climbing"),
    Dead        UMETA(DisplayName = "Dead")
};

// Combat state enum for weapon/tool context
UENUM(BlueprintType)
enum class EAnim_PlayerCombatState : uint8
{
    Unarmed     UMETA(DisplayName = "Unarmed"),
    SpearReady  UMETA(DisplayName = "Spear Ready"),
    BowReady    UMETA(DisplayName = "Bow Ready"),
    ClubReady   UMETA(DisplayName = "Club Ready"),
    Torch       UMETA(DisplayName = "Torch")
};

/**
 * UPlayerAnimInstance
 * Animation Blueprint logic for the prehistoric human player character.
 * Drives locomotion blend spaces, foot IK, aim offsets, and survival state reactions.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPlayerAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPlayerAnimInstance();

    // --- UAnimInstance interface ---
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // =========================================================
    // LOCOMOTION PROPERTIES
    // =========================================================

    /** Raw movement speed (cm/s) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float Speed;

    /** Smoothed speed for blend space — avoids snapping */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float SmoothedSpeed;

    /** Strafe direction in degrees (-180 to 180) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float Direction;

    /** True when character velocity > 3 cm/s */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsMoving;

    /** True when player holds sprint input and stamina > 0 */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsSprinting;

    /** True when crouched */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsCrouching;

    /** True when airborne (falling or jumping) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsInAir;

    /** Lateral lean amount for turn animations (-1 to 1) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float LeanAmount;

    // =========================================================
    // SURVIVAL STATE PROPERTIES
    // =========================================================

    /** True when player is in stealth/sneak mode */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    bool bIsSneaking;

    /** True when player is climbing a surface */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    bool bIsClimbing;

    /** Fear level 0-1: affects idle fidgets, breathing, posture */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float FearLevel;

    /** Stamina level 0-1: affects run speed blend and recovery pose */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Survival")
    float StaminaLevel;

    // =========================================================
    // COMBAT PROPERTIES
    // =========================================================

    /** True during attack montage window */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsAttacking;

    /** True during block/parry window */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsBlocking;

    /** True when hit reaction is playing */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsHurt;

    /** True when death animation is active */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsDead;

    /** Aim pitch offset for upper body aim (-90 to 90) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    float AimPitch;

    /** Aim yaw offset for upper body aim (-180 to 180) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    float AimYaw;

    // =========================================================
    // FOOT IK PROPERTIES
    // =========================================================

    /** IK blend alpha for left foot (0 = disabled, 1 = full IK) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    float LeftFootIKAlpha;

    /** IK blend alpha for right foot */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    float RightFootIKAlpha;

    /** Left foot world-space IK effector offset */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    FVector LeftFootEffectorLocation;

    /** Right foot world-space IK effector offset */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
    FVector RightFootEffectorLocation;

    // =========================================================
    // STATE MACHINE ENUMS
    // =========================================================

    /** Current movement state — drives state machine transitions */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|States")
    EAnim_PlayerMovementState MovementState;

    /** Current combat/weapon state — drives upper body layer */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|States")
    EAnim_PlayerCombatState CombatState;

    // =========================================================
    // CALLABLE FUNCTIONS (from Character or Ability System)
    // =========================================================

    UFUNCTION(BlueprintCallable, Category = "Anim|Control")
    void SetSprinting(bool bSprinting);

    UFUNCTION(BlueprintCallable, Category = "Anim|Control")
    void SetSneaking(bool bSneaking);

    UFUNCTION(BlueprintCallable, Category = "Anim|Control")
    void SetClimbing(bool bClimbing);

    UFUNCTION(BlueprintCallable, Category = "Anim|Control")
    void SetFearLevel(float Fear);

    UFUNCTION(BlueprintCallable, Category = "Anim|Control")
    void SetStaminaLevel(float Stamina);

    UFUNCTION(BlueprintCallable, Category = "Anim|Control")
    void SetCombatState(EAnim_PlayerCombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Anim|Control")
    void TriggerAttack();

    UFUNCTION(BlueprintCallable, Category = "Anim|Control")
    void TriggerHurt();

    UFUNCTION(BlueprintCallable, Category = "Anim|Control")
    void TriggerDeath();

private:
    // Cached references
    UPROPERTY()
    ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    // Internal update methods
    void UpdateLocomotionData(float DeltaSeconds);
    void UpdateAimData();
    void UpdateFootIK();
    void UpdateMovementState();
    void UpdateCombatState();

    /** Line-trace foot IK offset calculation */
    FVector CalculateFootIKOffset(const FName& FootSocketName);
};
