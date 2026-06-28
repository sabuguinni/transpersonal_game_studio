#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DinosaurAnimInstance.generated.h"

/**
 * UDinosaurAnimInstance
 * AnimInstance subclass for all dinosaur pawns.
 * Drives locomotion blend space, attack montages, and death/ragdoll transitions.
 * Used by: TRex, Raptor, Brachiosaurus, and any future dino species.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UDinosaurAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UDinosaurAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// ─── Locomotion ───────────────────────────────────────────────────────────

	/** Ground speed in cm/s, drives the locomotion blend space X axis */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	float GroundSpeed;

	/** Movement direction in degrees relative to actor forward (-180..180) */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	float MovementDirection;

	/** True while the dino is airborne (jump / fall) */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsInAir;

	/** True when the dino is actively chasing a target */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsChasing;

	/** True when the dino is in idle patrol mode */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Locomotion")
	bool bIsPatrolling;

	// ─── Combat ───────────────────────────────────────────────────────────────

	/** True when the dino is within melee strike range */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
	bool bIsAttacking;

	/** True when the dino has taken a lethal hit and is dying */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
	bool bIsDying;

	/** Normalised health (0..1) — used to blend wounded locomotion */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
	float HealthNormalized;

	/** True when the dino is roaring (aggression / territory warning) */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Combat")
	bool bIsRoaring;

	// ─── Foot IK ──────────────────────────────────────────────────────────────

	/** Left foot IK target offset in world space */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
	FVector LeftFootIKTarget;

	/** Right foot IK target offset in world space */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
	FVector RightFootIKTarget;

	/** Pelvis vertical offset to keep the body level on uneven terrain */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|FootIK")
	float PelvisOffset;

	// ─── Species ──────────────────────────────────────────────────────────────

	/** Species tag — used to select the correct blend space asset at runtime */
	UPROPERTY(BlueprintReadOnly, Category = "Anim|Species")
	FName SpeciesTag;

	/** Walk speed threshold (cm/s) — below this the dino plays idle */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Species")
	float WalkThreshold;

	/** Run speed threshold (cm/s) — above this the dino plays full sprint */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Species")
	float RunThreshold;

	// ─── Blueprint events ─────────────────────────────────────────────────────

	/** Called from BT when the dino should play its roar montage */
	UFUNCTION(BlueprintCallable, Category = "Anim|Combat")
	void TriggerRoar();

	/** Called from BT when the dino should play its attack montage */
	UFUNCTION(BlueprintCallable, Category = "Anim|Combat")
	void TriggerAttack();

	/** Called when health reaches 0 — transitions to death state */
	UFUNCTION(BlueprintCallable, Category = "Anim|Combat")
	void TriggerDeath();

private:
	/** Cached owning pawn */
	UPROPERTY()
	class APawn* OwnerPawn;

	/** Cached movement component */
	UPROPERTY()
	class UCharacterMovementComponent* MovementComp;

	/** Smooth interpolation of GroundSpeed to avoid jitter */
	float SmoothedSpeed;

	/** Internal timer for roar cooldown */
	float RoarCooldown;

	/** Update locomotion variables from movement component */
	void UpdateLocomotion(float DeltaSeconds);

	/** Update foot IK targets via line traces */
	void UpdateFootIK();

	/** Compute movement direction angle relative to actor forward */
	float ComputeMovementDirection() const;
};
