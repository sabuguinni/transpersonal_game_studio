#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalAnimInstance.generated.h"

/**
 * Animation state enum for the prehistoric survivor character.
 * Drives the AnimBlueprint state machine transitions.
 */
UENUM(BlueprintType)
enum class EAnim_CharacterState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walk        UMETA(DisplayName = "Walk"),
    Run         UMETA(DisplayName = "Run"),
    Crouch      UMETA(DisplayName = "Crouch"),
    Jump        UMETA(DisplayName = "Jump"),
    Fall        UMETA(DisplayName = "Fall"),
    Attack      UMETA(DisplayName = "Attack"),
    Death       UMETA(DisplayName = "Death")
};

/**
 * Foot IK data for terrain-adaptive foot placement.
 */
USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    /** Left foot world-space target location */
    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    FVector LeftFootTarget = FVector::ZeroVector;

    /** Right foot world-space target location */
    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    FVector RightFootTarget = FVector::ZeroVector;

    /** Left foot IK alpha (0=off, 1=full IK) */
    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    float LeftFootAlpha = 0.0f;

    /** Right foot IK alpha (0=off, 1=full IK) */
    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    float RightFootAlpha = 0.0f;

    /** Pelvis vertical offset to prevent foot stretching on slopes */
    UPROPERTY(BlueprintReadWrite, Category = "Animation|IK")
    float PelvisOffset = 0.0f;
};

/**
 * UTranspersonalAnimInstance
 * 
 * AnimInstance for the prehistoric survivor player character.
 * Drives locomotion blend space, state transitions, and foot IK.
 * 
 * Key systems:
 * - Speed-based locomotion (Idle → Walk → Run)
 * - Direction-aware strafe blending
 * - Airborne detection (Jump / Fall)
 * - Crouch state toggle
 * - Foot IK for terrain adaptation
 * - Attack montage integration
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UTranspersonalAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UTranspersonalAnimInstance();

    /** Called every frame to update animation variables from character state */
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    /** Called on initialization */
    virtual void NativeInitializeAnimation() override;

    // ── Locomotion Variables ──────────────────────────────────────────────

    /** Current movement speed (cm/s) — drives locomotion blend space */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    float Speed = 0.0f;

    /** Movement direction relative to character facing (-180 to 180 degrees) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    float Direction = 0.0f;

    /** True when character is in the air (jumping or falling) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    bool bIsInAir = false;

    /** True when character is crouching */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    bool bIsCrouching = false;

    /** True when character is sprinting (speed > RunThreshold) */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Locomotion",
              meta = (AllowPrivateAccess = "true"))
    bool bIsSprinting = false;

    /** Current high-level animation state */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|State",
              meta = (AllowPrivateAccess = "true"))
    EAnim_CharacterState CurrentState = EAnim_CharacterState::Idle;

    // ── Foot IK ───────────────────────────────────────────────────────────

    /** Foot IK data updated each frame via line traces */
    UPROPERTY(BlueprintReadOnly, Category = "Animation|IK",
              meta = (AllowPrivateAccess = "true"))
    FAnim_FootIKData FootIKData;

    /** Enable/disable foot IK (disable on steep slopes or in air) */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation|IK")
    bool bEnableFootIK = true;

    /** Trace distance below foot for IK ground detection */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation|IK")
    float FootTraceDistance = 50.0f;

    // ── Thresholds ────────────────────────────────────────────────────────

    /** Speed threshold to transition from Idle to Walk */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation|Thresholds")
    float WalkThreshold = 10.0f;

    /** Speed threshold to transition from Walk to Run */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation|Thresholds")
    float RunThreshold = 300.0f;

    // ── Blueprint Events ──────────────────────────────────────────────────

    /** Called when character lands after being airborne */
    UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Events")
    void OnLanded();

    /** Called when attack montage should play */
    UFUNCTION(BlueprintCallable, Category = "Animation|Combat")
    void TriggerAttackMontage();

    /** Returns true if character is moving on the ground */
    UFUNCTION(BlueprintPure, Category = "Animation|Locomotion")
    bool IsMovingOnGround() const;

private:
    /** Cached reference to the owning character */
    UPROPERTY()
    class ACharacter* OwnerCharacter = nullptr;

    /** Previous airborne state for landing detection */
    bool bWasInAir = false;

    /** Update foot IK via line traces */
    void UpdateFootIK(float DeltaSeconds);

    /** Compute current animation state from locomotion variables */
    EAnim_CharacterState ComputeAnimState() const;

    /** Smooth interpolation target for pelvis offset */
    float TargetPelvisOffset = 0.0f;
};
