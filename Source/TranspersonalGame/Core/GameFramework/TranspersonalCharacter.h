#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TranspersonalCharacter.generated.h"

// Forward declarations
class UCameraComponent;
class USpringArmComponent;
class USurvivalComponent;

/**
 * ATranspersonalCharacter
 * Player character for the prehistoric survival game.
 * Inherits from ACharacter for full movement/physics support.
 * Integrates SurvivalComponent for hunger/thirst/stamina/health tracking.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATranspersonalCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATranspersonalCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	virtual void Tick(float DeltaTime) override;

	// ─── Camera ───────────────────────────────────────────────────────────────

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	// ─── Survival Component ───────────────────────────────────────────────────

	/** Survival stats: health, hunger, thirst, stamina, fear */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
	USurvivalComponent* SurvivalComp;

	// ─── Movement Parameters ──────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RunSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float CrouchSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float JumpVelocity = 420.0f;

	/** True when the player is holding the sprint key */
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsSprinting = false;

	/** True when the player is crouching */
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsCrouching = false;

	// ─── Survival State ───────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float MaxHealth = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Survival")
	float CurrentHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float MaxStamina = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Survival")
	float CurrentStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float MaxHunger = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Survival")
	float CurrentHunger = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float MaxThirst = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Survival")
	float CurrentThirst = 100.0f;

	/** Fear level 0-100: increases near predators, decreases in safe zones */
	UPROPERTY(BlueprintReadOnly, Category = "Survival")
	float FearLevel = 0.0f;

	// ─── Input Handlers ───────────────────────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void MoveForward(float Value);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void MoveRight(float Value);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void Turn(float Value);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void LookUp(float Value);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartSprint();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopSprint();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void ToggleCrouch();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartJump();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopJump();

	// ─── Survival Functions ───────────────────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Survival")
	void ApplyDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Survival")
	void Eat(float NutritionValue);

	UFUNCTION(BlueprintCallable, Category = "Survival")
	void Drink(float HydrationValue);

	UFUNCTION(BlueprintCallable, Category = "Survival")
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, Category = "Survival")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintCallable, Category = "Survival")
	float GetStaminaPercent() const;

	UFUNCTION(BlueprintCallable, Category = "Survival")
	float GetHungerPercent() const;

	UFUNCTION(BlueprintCallable, Category = "Survival")
	float GetThirstPercent() const;

	// ─── Events ───────────────────────────────────────────────────────────────

	UFUNCTION(BlueprintImplementableEvent, Category = "Survival")
	void OnDeath();

	UFUNCTION(BlueprintImplementableEvent, Category = "Survival")
	void OnNearStarvation();

	UFUNCTION(BlueprintImplementableEvent, Category = "Survival")
	void OnNearDehydration();

private:
	/** Decay survival stats over time */
	void TickSurvivalStats(float DeltaTime);

	/** Drain stamina while sprinting, recover when idle */
	void TickStamina(float DeltaTime);

	/** Rate at which hunger decreases per second */
	float HungerDecayRate = 0.5f;

	/** Rate at which thirst decreases per second */
	float ThirstDecayRate = 0.8f;

	/** Stamina drain rate while sprinting (per second) */
	float StaminaDrainRate = 15.0f;

	/** Stamina recovery rate when not sprinting (per second) */
	float StaminaRecoveryRate = 8.0f;

	/** Health damage per second when starving */
	float StarvationDamageRate = 2.0f;

	/** Health damage per second when dehydrated */
	float DehydrationDamageRate = 3.0f;

	bool bIsDead = false;
};
