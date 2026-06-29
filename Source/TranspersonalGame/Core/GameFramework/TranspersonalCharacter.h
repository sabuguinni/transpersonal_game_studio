#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TranspersonalCharacter.generated.h"

// Forward declarations
class USpringArmComponent;
class UCameraComponent;
class USurvivalComponent;

/**
 * TranspersonalCharacter — Playable prehistoric human survivor.
 * Handles WASD movement, camera boom, jump, run, and survival stats.
 * Uses standard UE5 ACharacter + UCharacterMovementComponent.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATranspersonalCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATranspersonalCharacter();

	// === CAMERA ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	// === SURVIVAL COMPONENT ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
	USurvivalComponent* SurvivalComp;

	// === MOVEMENT PARAMETERS ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RunSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float JumpVelocity;

	// === SURVIVAL STATS (replicated from SurvivalComp for HUD) ===
	UPROPERTY(BlueprintReadOnly, Category = "Survival")
	float Health;

	UPROPERTY(BlueprintReadOnly, Category = "Survival")
	float Hunger;

	UPROPERTY(BlueprintReadOnly, Category = "Survival")
	float Thirst;

	UPROPERTY(BlueprintReadOnly, Category = "Survival")
	float Stamina;

	UPROPERTY(BlueprintReadOnly, Category = "Survival")
	float Fear;

	// === STATE FLAGS ===
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsSprinting;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsCrouching;

	// === INPUT ACTIONS ===
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartSprint();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopSprint();

	UFUNCTION(BlueprintCallable, Category = "Survival")
	void Eat(float NutritionValue);

	UFUNCTION(BlueprintCallable, Category = "Survival")
	void Drink(float HydrationValue);

	UFUNCTION(BlueprintCallable, Category = "Survival")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintCallable, Category = "Survival")
	float GetStaminaPercent() const;

	UFUNCTION(BlueprintCallable, Category = "Survival")
	bool IsAlive() const;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// Input handlers
	void MoveForward(float Value);
	void MoveRight(float Value);
	void LookUp(float Value);
	void LookRight(float Value);
	void OnJumpPressed();
	void OnJumpReleased();
	void OnCrouchToggle();

	// Survival tick
	void TickSurvivalStats(float DeltaTime);

private:
	// Stamina drain timer
	float StaminaDrainRate;
	float StaminaRegenRate;
	float HungerDrainRate;
	float ThirstDrainRate;
};
