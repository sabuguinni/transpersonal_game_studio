// TRexCharacter.h
// Core Systems Programmer #03 — Cycle PROD_CYCLE_AUTO_20260620_003
// Tyrannosaurus Rex — apex predator subclass of ADinosaurBase
// Inherits full AI state machine, sensory system, and stamina from base

#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TRexCharacter.generated.h"

/**
 * ATRexCharacter
 * Tyrannosaurus Rex — the apex predator of the prehistoric world.
 * Massive carnivore with devastating bite, limited turning radius, and
 * exceptional hearing to compensate for forward-only vision.
 *
 * Stats (tuned for gameplay balance):
 *   Walk: 250 cm/s  Run: 750 cm/s  Attack: 150 dmg  HP: 1200
 *   Detection: 4000 cm radius  FOV: 90°  Hearing: 3500 cm
 *   Roar cooldown: 15s — stuns player for 1.5s within 1200 cm
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATRexCharacter : public ADinosaurBase
{
	GENERATED_BODY()

public:
	ATRexCharacter();

	// --- Roar ability ---

	/** Radius within which the roar stun effect applies (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
	float RoarStunRadius;

	/** Duration of player stun from roar (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
	float RoarStunDuration;

	/** Cooldown between roars (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
	float RoarCooldown;

	/** Whether the roar is currently on cooldown */
	UPROPERTY(BlueprintReadOnly, Category = "TRex|Abilities")
	bool bRoarOnCooldown;

	/** Perform roar — stuns nearby players and alerts all dinos in radius */
	UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
	void PerformRoar();

	/** Called when roar cooldown expires */
	UFUNCTION()
	void OnRoarCooldownExpired();

	// --- Stomp attack ---

	/** Radius of ground stomp AoE (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
	float StompRadius;

	/** Damage dealt by stomp AoE */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
	float StompDamage;

	/** Perform stomp — AoE damage around feet */
	UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
	void PerformStomp();

	// --- TRex-specific overrides ---

	/** TRex has limited turn rate — override to enforce max yaw rate */
	virtual void Tick(float DeltaTime) override;

	/** Override to add roar on first aggro */
	virtual void OnTargetDetected(AActor* Target) override;

protected:
	virtual void BeginPlay() override;

private:
	/** Timer handle for roar cooldown */
	FTimerHandle RoarCooldownTimer;

	/** Max yaw rotation rate (deg/s) — TRex turns slowly */
	float MaxTurnRateDegPerSec;
};
