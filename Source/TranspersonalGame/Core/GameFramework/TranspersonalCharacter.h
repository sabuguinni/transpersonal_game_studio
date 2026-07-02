#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TranspersonalCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USurvivalComponent;

/**
 * ATranspersonalCharacter
 * Main playable character for the prehistoric survival game.
 * Supports WASD movement, sprint, jump, and survival stats via SurvivalComponent.
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

	// ── Camera ──────────────────────────────────────────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	// ── Survival ─────────────────────────────────────────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival", meta = (AllowPrivateAccess = "true"))
	USurvivalComponent* SurvivalComp;

	// ── Movement ─────────────────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeed = 750.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float JumpZVelocity = 420.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsSprinting = false;

	// ── Input handlers ───────────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void MoveForward(float Value);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void MoveRight(float Value);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void LookUp(float Value);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void Turn(float Value);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartSprint();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopSprint();

	// ── Survival queries ─────────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Survival")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Survival")
	float GetHunger() const;

	UFUNCTION(BlueprintCallable, Category = "Survival")
	float GetThirst() const;

	UFUNCTION(BlueprintCallable, Category = "Survival")
	float GetStamina() const;

	UFUNCTION(BlueprintCallable, Category = "Survival")
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, Category = "Survival")
	void ApplyDamage_Survival(float DamageAmount);
};
