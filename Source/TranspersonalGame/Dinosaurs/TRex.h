#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TRex.generated.h"

/**
 * ATRex — Tyrannosaurus Rex
 * Apex predator. Carnivore. Ambush-capable, high damage, short detection arc.
 * Inherits full AI state machine from ADinosaurBase.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATRex : public ADinosaurBase
{
	GENERATED_BODY()

public:
	ATRex();

	/** Roar ability — triggers fear on nearby players, cooldown 15s */
	UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
	void PerformRoar();

	/** Charge attack — sprint burst toward target if within 800cm */
	UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
	void ChargeAttack();

	/** Whether TRex is currently in charge state */
	UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
	bool bIsCharging;

	/** Fear radius — players within this range receive fear debuff */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
	float RoarFearRadius;

	/** Charge speed multiplier applied during charge attack */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
	float ChargeSpeedMultiplier;

	/** Cooldown timer handle for roar */
	FTimerHandle RoarCooldownHandle;

	/** Whether roar is on cooldown */
	bool bRoarOnCooldown;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** Override death — TRex collapses with dramatic effect */
	virtual void OnDinosaurDeath_Implementation() override;

	/** Override detection — TRex has narrow forward arc but extreme range */
	virtual void OnPlayerDetected_Implementation(AActor* Player, float Distance) override;

private:
	void ResetRoarCooldown();
	float ChargeElapsed;
	float ChargeDuration;
};
