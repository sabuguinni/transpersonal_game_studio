#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimInstance.generated.h"

// ============================================================
// Enums — must be at global scope (UHT rule)
// ============================================================

UENUM(BlueprintType)
enum class EAnim_LocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EAnim_StanceType : uint8
{
    Standing    UMETA(DisplayName = "Standing"),
    Crouched    UMETA(DisplayName = "Crouched"),
    Prone       UMETA(DisplayName = "Prone")
};

// ============================================================
// UTranspersonalAnimInstance
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UTranspersonalAnimInstance();

    // Called every frame to update animation variables
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    virtual void NativeInitializeAnimation() override;

    // --------------------------------------------------------
    // Locomotion State
    // --------------------------------------------------------

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    EAnim_LocomotionState LocomotionState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    EAnim_StanceType StanceType;

    /** Horizontal ground speed (cm/s) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    float Speed;

    /** Lateral direction offset for strafing blend (-180 to 180) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    float Direction;

    /** Lean angle for banking during turns */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    float LeanAngle;

    /** Vertical velocity — positive = rising, negative = falling */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    float VerticalVelocity;

    /** True when the character is in the air */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsInAir;

    /** True when crouching */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsCrouching;

    /** True when sprinting (speed > SprintThreshold) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsSprinting;

    /** True when the character is alive */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsAlive;

    // --------------------------------------------------------
    // Foot IK
    // --------------------------------------------------------

    /** Left foot world-space effector location for terrain IK */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|FootIK")
    FVector LeftFootEffectorLocation;

    /** Right foot world-space effector location for terrain IK */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|FootIK")
    FVector RightFootEffectorLocation;

    /** Left foot normal rotation from surface trace */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|FootIK")
    FRotator LeftFootRotation;

    /** Right foot normal rotation from surface trace */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|FootIK")
    FRotator RightFootRotation;

    /** Pelvis offset to keep both feet grounded on uneven terrain */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|FootIK")
    float PelvisOffset;

    /** True when foot IK is active (only when on ground and moving slowly) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|FootIK")
    bool bFootIKEnabled;

    // --------------------------------------------------------
    // Survival State (drives additive layers)
    // --------------------------------------------------------

    /** 0.0 = full health, 1.0 = near death — drives wounded additive */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Survival")
    float WoundedBlendWeight;

    /** 0.0 = rested, 1.0 = exhausted — drives heavy breathing additive */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Survival")
    float ExhaustionBlendWeight;

    /** 0.0 = warm, 1.0 = freezing — drives shiver additive */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim|Survival")
    float ColdBlendWeight;

    // --------------------------------------------------------
    // Speed thresholds (configurable per character)
    // --------------------------------------------------------

    /** Speed above which the character is considered walking */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim|Config")
    float WalkThreshold;

    /** Speed above which the character is considered sprinting */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anim|Config")
    float SprintThreshold;

protected:
    /** Cached owner character */
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    /** Cached movement component */
    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

private:
    /** Update locomotion variables from movement component */
    void UpdateLocomotion(float DeltaSeconds);

    /** Perform two-bone IK foot traces and update foot properties */
    void UpdateFootIK(float DeltaSeconds);

    /** Update survival-driven additive blend weights from character stats */
    void UpdateSurvivalBlends(float DeltaSeconds);

    /** Determine locomotion state from speed and flags */
    EAnim_LocomotionState DetermineLocomotionState() const;

    /** Smooth interpolation target for lean */
    float LeanAngleTarget;

    /** Previous frame velocity for lean calculation */
    FVector PreviousVelocity;

    /** Foot IK trace channel */
    static constexpr float FootIKTraceDistance = 50.0f;
    static constexpr float FootIKInterpSpeed   = 15.0f;
    static constexpr float PelvisInterpSpeed   = 10.0f;
};
