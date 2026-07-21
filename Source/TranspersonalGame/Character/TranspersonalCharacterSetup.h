#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "TranspersonalCharacterSetup.generated.h"

// ============================================================
// Char_SurvivalStats — survival statistics for the player
// ============================================================
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_SurvivalStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float Hunger = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float MaxHunger = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float Thirst = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float MaxThirst = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float Stamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float MaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float Fear = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float MaxFear = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float BodyTemperature = 37.0f;
};

// ============================================================
// EChar_MovementState — player movement states
// ============================================================
UENUM(BlueprintType)
enum class EChar_MovementState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Walking		UMETA(DisplayName = "Walking"),
	Running		UMETA(DisplayName = "Running"),
	Crouching	UMETA(DisplayName = "Crouching"),
	Jumping		UMETA(DisplayName = "Jumping"),
	Climbing	UMETA(DisplayName = "Climbing"),
	Swimming	UMETA(DisplayName = "Swimming"),
	Dead		UMETA(DisplayName = "Dead")
};

// ============================================================
// EChar_StanceType — player combat/survival stance
// ============================================================
UENUM(BlueprintType)
enum class EChar_StanceType : uint8
{
	Neutral		UMETA(DisplayName = "Neutral"),
	Combat		UMETA(DisplayName = "Combat"),
	Stealth		UMETA(DisplayName = "Stealth"),
	Foraging	UMETA(DisplayName = "Foraging"),
	Crafting	UMETA(DisplayName = "Crafting")
};

// ============================================================
// ATranspersonalCharacterSetup — playable prehistoric survivor
// Third-person character with WASD movement, camera boom,
// survival stats, and stamina-based sprint system.
// ============================================================
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATranspersonalCharacterSetup : public ACharacter
{
	GENERATED_BODY()

public:
	ATranspersonalCharacterSetup();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ── Camera ──────────────────────────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	// ── Survival Stats ───────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	FChar_SurvivalStats SurvivalStats;

	// ── Movement State ───────────────────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	EChar_MovementState CurrentMovementState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	EChar_StanceType CurrentStance;

	// ── Movement Config ──────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RunSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float CrouchSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float StaminaDrainRate = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float StaminaRegenRate = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float HungerDrainRate = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float ThirstDrainRate = 0.8f;

	// ── Sprint State ─────────────────────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsSprinting = false;

	// ── Camera Config ────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraBoomLength = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FVector CameraSocketOffset = FVector(0.0f, 0.0f, 80.0f);

public:
	// ── Input Handlers ───────────────────────────────────────
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
	void StartCrouch();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopCrouch();

	// ── Survival API ─────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Survival")
	void ApplyDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Survival")
	void Eat(float NutritionValue);

	UFUNCTION(BlueprintCallable, Category = "Survival")
	void Drink(float HydrationValue);

	UFUNCTION(BlueprintPure, Category = "Survival")
	bool IsAlive() const;

	UFUNCTION(BlueprintPure, Category = "Survival")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Survival")
	float GetHungerPercent() const;

	UFUNCTION(BlueprintPure, Category = "Survival")
	float GetThirstPercent() const;

	UFUNCTION(BlueprintPure, Category = "Survival")
	float GetStaminaPercent() const;

	UFUNCTION(BlueprintPure, Category = "Survival")
	FChar_SurvivalStats GetSurvivalStats() const { return SurvivalStats; }

	// ── Getters ──────────────────────────────────────────────
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE EChar_MovementState GetMovementState() const { return CurrentMovementState; }

private:
	void UpdateSurvivalStats(float DeltaTime);
	void UpdateMovementState();
	void UpdateStamina(float DeltaTime);
};
