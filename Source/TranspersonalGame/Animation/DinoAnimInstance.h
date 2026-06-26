#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinoAnimInstance.generated.h"

/**
 * Animation instance for dinosaur skeletal meshes.
 * Drives locomotion blend space, attack montages, and idle variation.
 * Agent #10 — Animation Agent
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

    /** Current movement speed (0 = idle, >0 = walking/running) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float Speed;

    /** Lateral strafe direction (-1 left, 0 straight, 1 right) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    float Direction;

    /** True when the dino is airborne (jump/fall) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsInAir;

    /** True when the dino is actively sprinting */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
    bool bIsSprinting;

    // ── Combat ──────────────────────────────────────────────────────────────

    /** True when the dino is attacking */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsAttacking;

    /** True when the dino has been hit */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsHit;

    /** True when the dino is dead */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
    bool bIsDead;

    // ── Behaviour ───────────────────────────────────────────────────────────

    /** True when the dino is eating */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Behaviour")
    bool bIsEating;

    /** True when the dino is roaring */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Behaviour")
    bool bIsRoaring;

    /** Normalised alert level 0-1 (0=calm, 1=fully alerted) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Behaviour")
    float AlertLevel;

    // ── IK ──────────────────────────────────────────────────────────────────

    /** Left foot IK target world location */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector LeftFootIKLocation;

    /** Right foot IK target world location */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    FVector RightFootIKLocation;

    /** IK alpha blend (0 = off, 1 = full IK) */
    UPROPERTY(BlueprintReadOnly, Category = "Anim|IK")
    float IKAlpha;

    // ── Trigger functions (called by AI/combat systems) ──────────────────────

    UFUNCTION(BlueprintCallable, Category = "Anim|Combat")
    void TriggerAttack();

    UFUNCTION(BlueprintCallable, Category = "Anim|Combat")
    void TriggerHit();

    UFUNCTION(BlueprintCallable, Category = "Anim|Combat")
    void TriggerDeath();

    UFUNCTION(BlueprintCallable, Category = "Anim|Behaviour")
    void TriggerRoar();

private:
    /** Cached owning pawn */
    UPROPERTY()
    class APawn* OwnerPawn;

    /** Cached movement component */
    UPROPERTY()
    class UMovementComponent* MovementComp;

    /** Update foot IK positions using line traces */
    void UpdateFootIK(float DeltaSeconds);

    /** Smooth IK alpha in/out based on movement speed */
    void UpdateIKAlpha(float DeltaSeconds);

    float IKAlphaTarget;
    float IKAlphaInterpSpeed;
};
