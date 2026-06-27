#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TyrannosaurusRex.generated.h"

/**
 * ATyrannosaurusRex
 * Apex predator. Solitary carnivore. High health, devastating attack, large detection radius.
 * Slow turn rate but charges at high speed when attacking.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATyrannosaurusRex : public ADinosaurBase
{
	GENERATED_BODY()

public:
	ATyrannosaurusRex();

	/** Roar ability — stuns nearby prey for a short duration */
	UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
	void PerformRoar();

	/** Charge attack — accelerates toward target, deals bonus damage on impact */
	UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
	void StartCharge();

protected:
	virtual void BeginPlay() override;
	virtual void PerformAttack(AActor* Target) override;
	virtual void OnDeath() override;

	/** Duration of roar stun effect on nearby actors (seconds) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TRex|Abilities")
	float RoarStunDuration;

	/** Charge speed multiplier applied during charge attack */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TRex|Abilities")
	float ChargeSpeedMultiplier;

	/** Bonus damage dealt when charge connects */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TRex|Abilities")
	float ChargeBonusDamage;

	/** Radius of roar stun effect */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TRex|Abilities")
	float RoarRadius;

private:
	bool bIsCharging;
	float ChargeTimer;
	static constexpr float MaxChargeDuration = 3.0f;
};
