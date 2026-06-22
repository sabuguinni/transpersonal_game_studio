#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Core/Survival/SurvivalComponent.h"
#include "TranspersonalCharacter.generated.h"

// Forward declarations
class UCameraComponent;
class USpringArmComponent;
class ABiomeManager;

/**
 * ATranspersonalCharacter
 * Main playable character for the prehistoric survival game.
 * Handles WASD movement, camera, and survival stat integration via SurvivalComponent.
 * BiomeManager modifiers are applied each survival tick via GetSurvivalModifiers().
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATranspersonalCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATranspersonalCharacter();

	// --- Camera ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	// --- Survival ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival")
	USurvivalComponent* SurvivalComp;

	// --- Movement ---
	/** Walk speed (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 400.f;

	/** Sprint speed (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeed = 800.f;

	/** Whether the character is currently sprinting */
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsSprinting = false;

	// --- Biome Integration ---
	/** Cached reference to the world BiomeManager (found at BeginPlay) */
	UPROPERTY(BlueprintReadOnly, Category = "World")
	ABiomeManager* BiomeManagerRef;

	/** How often (seconds) to query biome modifiers and apply to survival stats */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float BiomeTickInterval = 5.0f;

	// --- Public API ---
	/** Start sprinting */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartSprint();

	/** Stop sprinting */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopSprint();

	/** Returns current health (0-100) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival")
	float GetHealth() const;

	/** Returns current hunger (0-100, 0 = starving) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival")
	float GetHunger() const;

	/** Returns current thirst (0-100, 0 = dehydrated) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival")
	float GetThirst() const;

	/** Returns current stamina (0-100) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival")
	float GetStamina() const;

	/** Returns current fear level (0-100) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival")
	float GetFear() const;

	/** Apply damage to the character (routes through SurvivalComponent) */
	UFUNCTION(BlueprintCallable, Category = "Survival")
	void ApplyDamage_Survival(float DamageAmount);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	// Input handlers
	void MoveForward(float Value);
	void MoveRight(float Value);
	void LookUp(float Value);
	void LookRight(float Value);
	void Jump_Input();
	void StopJump_Input();

	// Biome modifier application
	float BiomeTickAccumulator = 0.f;
	void ApplyBiomeModifiers();

	// Sprint stamina drain
	void HandleSprintStamina(float DeltaTime);
};
