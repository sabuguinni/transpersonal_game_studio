#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoAnimInstance.generated.h"

/**
 * Animation instance for dinosaur characters.
 * Drives locomotion blend spaces, attack montages, and IK foot placement.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinoAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UDinoAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ── Locomotion ──────────────────────────────────────────────────────────

    /** Current movement speed (cm/s) used to drive the locomotion blend space */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
    float Speed;

    /** Lateral strafe direction (-1 left, 0 straight, 1 right) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
    float Direction;

    /** True when the pawn is airborne */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion", meta = (AllowPrivateAccess = "true"))
    bool bIsInAir;

    /** True when the pawn is in an attack state */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat", meta = (AllowPrivateAccess = "true"))
    bool bIsAttacking;

    /** True when the pawn is eating */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Behaviour", meta = (AllowPrivateAccess = "true"))
    bool bIsEating;

    /** True when the pawn is in an alert/threat-detection state */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Behaviour", meta = (AllowPrivateAccess = "true"))
    bool bIsAlert;

    // ── IK Foot Placement ───────────────────────────────────────────────────

    /** Left foot IK target location in world space */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK", meta = (AllowPrivateAccess = "true"))
    FVector LeftFootIKLocation;

    /** Right foot IK target location in world space */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK", meta = (AllowPrivateAccess = "true"))
    FVector RightFootIKLocation;

    /** Pelvis vertical offset to keep body centred over feet on slopes */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK", meta = (AllowPrivateAccess = "true"))
    float PelvisOffset;

    // ── Tail Physics ────────────────────────────────────────────────────────

    /** Normalised tail sway amount driven by speed (0-1) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Physics", meta = (AllowPrivateAccess = "true"))
    float TailSwayAlpha;

    // ── Public API ──────────────────────────────────────────────────────────

    /** Called by AI/combat system to trigger an attack montage */
    UFUNCTION(BlueprintCallable, Category = "Anim|Combat")
    void TriggerAttack();

    /** Called when the pawn starts eating */
    UFUNCTION(BlueprintCallable, Category = "Anim|Behaviour")
    void StartEating();

    /** Called when the pawn stops eating */
    UFUNCTION(BlueprintCallable, Category = "Anim|Behaviour")
    void StopEating();

private:
    /** Cached owner pawn */
    UPROPERTY()
    class APawn* OwnerPawn;

    /** Cached movement component */
    UPROPERTY()
    class UCharacterMovementComponent* MovementComp;

    /** Perform a line trace to find the ground position for IK */
    FVector TraceFootIK(FName SocketName) const;

    /** Smooth interpolation alpha for pelvis offset */
    float PelvisOffsetTarget;
};
