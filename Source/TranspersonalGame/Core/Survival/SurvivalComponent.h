#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComponent.generated.h"

/**
 * FSurvivalStats — snapshot of all survival stats for UI/save purposes.
 */
USTRUCT(BlueprintType)
struct FSurvivalStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Survival")
	float Health = 100.f;

	UPROPERTY(BlueprintReadWrite, Category = "Survival")
	float Hunger = 100.f;

	UPROPERTY(BlueprintReadWrite, Category = "Survival")
	float Thirst = 100.f;

	UPROPERTY(BlueprintReadWrite, Category = "Survival")
	float Stamina = 100.f;

	UPROPERTY(BlueprintReadWrite, Category = "Survival")
	float Temperature = 37.f;

	UPROPERTY(BlueprintReadWrite, Category = "Survival")
	float Fear = 0.f;
};

/**
 * USurvivalComponent — manages all survival stats for the player character.
 *
 * Attach to ATranspersonalCharacter in its constructor.
 * Ticks every second to drain hunger/thirst and apply temperature effects.
 */
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USurvivalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USurvivalComponent();

	// --- Drain rates (units per second) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
	float HungerDrainRate = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
	float ThirstDrainRate = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
	float StaminaRegenRate = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Rates")
	float HealthDrainWhenStarving = 1.f;

	// --- Current stats ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|Stats")
	float Health = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|Stats")
	float Hunger = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|Stats")
	float Thirst = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|Stats")
	float Stamina = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|Stats")
	float Temperature = 37.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival|Stats")
	float Fear = 0.f;

	// --- Thresholds ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
	float CriticalHungerThreshold = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Thresholds")
	float CriticalThirstThreshold = 10.f;

	// --- Public API ---

	/** Apply damage to health. Returns true if character dies. */
	UFUNCTION(BlueprintCallable, Category = "Survival")
	bool ApplyDamage(float DamageAmount);

	/** Consume food — restores hunger by Amount. */
	UFUNCTION(BlueprintCallable, Category = "Survival")
	void EatFood(float Amount);

	/** Drink water — restores thirst by Amount. */
	UFUNCTION(BlueprintCallable, Category = "Survival")
	void DrinkWater(float Amount);

	/** Drain stamina (sprinting, jumping). Returns false if stamina depleted. */
	UFUNCTION(BlueprintCallable, Category = "Survival")
	bool DrainStamina(float Amount);

	/** Set fear level (0-100). Called by DinosaurAI proximity system. */
	UFUNCTION(BlueprintCallable, Category = "Survival")
	void SetFear(float FearLevel);

	/** Get a snapshot of all stats. */
	UFUNCTION(BlueprintCallable, Category = "Survival")
	FSurvivalStats GetStats() const;

	/** Returns true if character is alive. */
	UFUNCTION(BlueprintCallable, Category = "Survival")
	bool IsAlive() const { return Health > 0.f; }

	/** Returns true if stamina is sufficient for sprinting. */
	UFUNCTION(BlueprintCallable, Category = "Survival")
	bool CanSprint() const { return Stamina > 10.f; }

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	/** Accumulator for tick-based drain (fires every 1 second). */
	float DrainAccumulator = 0.f;

	/** Apply per-second survival drain logic. */
	void ApplySurvivalDrain(float DeltaSeconds);

	/** Clamp all stats to [0, 100] range. */
	void ClampStats();
};
