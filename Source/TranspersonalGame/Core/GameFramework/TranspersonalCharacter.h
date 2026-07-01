#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Core/Survival/SurvivalComponent.h"
#include "TranspersonalCharacter.generated.h"

/**
 * ATranspersonalCharacter
 * Player character for the prehistoric survival game.
 * Inherits from ACharacter (WASD movement, jump, camera boom).
 * Integrates USurvivalComponent for hunger/thirst/stamina/health/fear tracking.
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

	// ── Survival Component ──────────────────────────────────────────────────
	/** Tracks health, hunger, thirst, stamina, and fear for the player */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival",
		meta = (AllowPrivateAccess = "true"))
	USurvivalComponent* SurvivalComp;

	// ── Camera ──────────────────────────────────────────────────────────────
	/** Spring arm for third-person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera",
		meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera",
		meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	// ── Movement ────────────────────────────────────────────────────────────
	/** Base walk speed (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 400.0f;

	/** Sprint speed (cm/s) — drains stamina */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeed = 700.0f;

	/** True while the player is holding the sprint key */
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsSprinting = false;

	// ── Input handlers ──────────────────────────────────────────────────────
	/** Move forward/backward */
	void MoveForward(float Value);
	/** Strafe left/right */
	void MoveRight(float Value);
	/** Look up/down (mouse Y) */
	void LookUp(float Value);
	/** Turn left/right (mouse X) */
	void Turn(float Value);
	/** Start sprinting */
	void StartSprint();
	/** Stop sprinting */
	void StopSprint();

	// ── Survival helpers ────────────────────────────────────────────────────
	/** Called when stamina reaches 0 — forces walk speed */
	UFUNCTION(BlueprintCallable, Category = "Survival")
	void OnStaminaDepleted();

	/** Returns current health (0-100) from SurvivalComp */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival")
	float GetHealth() const;

	/** Returns current hunger (0-100) from SurvivalComp */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival")
	float GetHunger() const;

	/** Returns current thirst (0-100) from SurvivalComp */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival")
	float GetThirst() const;

	/** Returns current stamina (0-100) from SurvivalComp */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival")
	float GetStamina() const;

	/** Returns current fear level (0-100) from SurvivalComp */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival")
	float GetFear() const;

private:
	/** Stamina drain rate per second while sprinting */
	static constexpr float SprintStaminaDrainRate = 10.0f;

	/** Stamina recovery rate per second while not sprinting */
	static constexpr float StaminaRecoveryRate = 5.0f;
};
