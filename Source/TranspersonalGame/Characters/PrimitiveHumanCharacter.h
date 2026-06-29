#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "SharedTypes.h"
#include "PrimitiveHumanCharacter.generated.h"

// Forward declarations
class USkeletalMeshComponent;
class UCapsuleComponent;
class UCharacterMovementComponent;

/**
 * Visual appearance profile for a primitive human character.
 * Defines the physical and cosmetic attributes of a Cretaceous era survivor.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_AppearanceProfile
{
	GENERATED_BODY()

	/** Character display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FString CharacterName;

	/** Body height in cm (range: 155-185) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "155.0", ClampMax = "185.0"))
	float HeightCm = 170.0f;

	/** Body muscle mass index (0.0 = lean, 1.0 = very muscular) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MuscleMassIndex = 0.5f;

	/** Number of visible scars from dinosaur encounters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0", ClampMax = "12"))
	int32 ScarCount = 0;

	/** Skin weathering level from sun exposure (0.0 = fresh, 1.0 = heavily weathered) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SkinWeatheringLevel = 0.3f;

	/** Hair length index (0=shaved, 1=short, 2=medium, 3=long, 4=wild matted) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0", ClampMax = "4"))
	int32 HairLengthIndex = 3;

	/** Tribal paint coverage (0.0 = none, 1.0 = full body markings) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TribalPaintCoverage = 0.2f;

	/** Whether character has bone ornaments in hair */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	bool bHasBoneOrnaments = false;

	/** Clothing material index (0=bare, 1=basic hide, 2=stitched leather, 3=full armor) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0", ClampMax = "3"))
	int32 ClothingTierIndex = 1;
};

/**
 * Survival stats for a primitive human character.
 * Tracks all vital metrics needed for Cretaceous era survival gameplay.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_SurvivalStats
{
	GENERATED_BODY()

	/** Current health (0.0 = dead, 100.0 = full health) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float Health = 100.0f;

	/** Maximum health (increases with experience and nutrition) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float MaxHealth = 100.0f;

	/** Hunger level (0.0 = starving, 100.0 = full) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float Hunger = 80.0f;

	/** Thirst level (0.0 = dehydrated, 100.0 = fully hydrated) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float Thirst = 80.0f;

	/** Stamina for sprinting and combat (0.0 = exhausted, 100.0 = full) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float Stamina = 100.0f;

	/** Body temperature in Celsius (ideal: 37.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float BodyTemperatureCelsius = 37.0f;

	/** Fear level from nearby predators (0.0 = calm, 1.0 = terrified — affects movement speed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FearLevel = 0.0f;

	/** Number of dinosaur encounters survived (affects fear resistance) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	int32 DinosaurEncountersSurvived = 0;
};

/**
 * APrimitiveHumanCharacter — The playable Cretaceous era survivor.
 * 
 * A third-person character with WASD movement, spring arm camera,
 * survival stats tracking, and visual appearance customization.
 * Inherits from ACharacter for full UE5 movement component support.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APrimitiveHumanCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APrimitiveHumanCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	virtual void Tick(float DeltaTime) override;

	// ===== CAMERA COMPONENTS =====

	/** Spring arm for third-person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	// ===== APPEARANCE =====

	/** Visual appearance profile for this character */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Appearance")
	FChar_AppearanceProfile AppearanceProfile;

	// ===== SURVIVAL STATS =====

	/** Current survival stats */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Survival")
	FChar_SurvivalStats SurvivalStats;

	/** Rate at which hunger decreases per second during normal activity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Survival")
	float HungerDecayRate = 0.02f;

	/** Rate at which thirst decreases per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Survival")
	float ThirstDecayRate = 0.03f;

	/** Rate at which stamina regenerates per second when not sprinting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Survival")
	float StaminaRegenRate = 5.0f;

	/** Stamina drain rate per second while sprinting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Survival")
	float StaminaSprintDrain = 10.0f;

	// ===== MOVEMENT =====

	/** Whether the character is currently sprinting */
	UPROPERTY(BlueprintReadOnly, Category = "Character|Movement")
	bool bIsSprinting = false;

	/** Walk speed (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Movement")
	float WalkSpeed = 300.0f;

	/** Sprint speed (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Movement")
	float SprintSpeed = 600.0f;

	/** Crouch walk speed (cm/s) — used for stealth near predators */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Movement")
	float CrouchWalkSpeed = 150.0f;

	// ===== NIGHT VISION =====

	/** Night vision stamina drain multiplier (from NightCycleManager) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Night")
	float NightVisionStaminaDrainMultiplier = 1.5f;

	/** Whether night vision mode is active */
	UPROPERTY(BlueprintReadOnly, Category = "Character|Night")
	bool bNightVisionActive = false;

	// ===== FUNCTIONS =====

	/** Apply damage to the character */
	UFUNCTION(BlueprintCallable, Category = "Character|Survival")
	void ApplyDamage(float DamageAmount);

	/** Consume food to restore hunger */
	UFUNCTION(BlueprintCallable, Category = "Character|Survival")
	void ConsumeFood(float NutritionValue);

	/** Drink water to restore thirst */
	UFUNCTION(BlueprintCallable, Category = "Character|Survival")
	void DrinkWater(float HydrationValue);

	/** Start sprinting */
	UFUNCTION(BlueprintCallable, Category = "Character|Movement")
	void StartSprint();

	/** Stop sprinting */
	UFUNCTION(BlueprintCallable, Category = "Character|Movement")
	void StopSprint();

	/** Get current fear level (0.0-1.0) */
	UFUNCTION(BlueprintPure, Category = "Character|Survival")
	float GetFearLevel() const { return SurvivalStats.FearLevel; }

	/** Set fear level from nearby predator detection */
	UFUNCTION(BlueprintCallable, Category = "Character|Survival")
	void SetFearLevel(float NewFearLevel);

	/** Check if character is alive */
	UFUNCTION(BlueprintPure, Category = "Character|Survival")
	bool IsAlive() const { return SurvivalStats.Health > 0.0f; }

	/** Get effective movement speed accounting for fear, hunger, and stamina */
	UFUNCTION(BlueprintPure, Category = "Character|Movement")
	float GetEffectiveMovementSpeed() const;

	/** Toggle night vision mode (drains stamina) */
	UFUNCTION(BlueprintCallable, Category = "Character|Night")
	void ToggleNightVision();

protected:
	/** Input: Move forward/backward */
	void MoveForward(float Value);

	/** Input: Move right/left */
	void MoveRight(float Value);

	/** Input: Look up/down */
	void LookUp(float Value);

	/** Input: Look right/left */
	void LookRight(float Value);

	/** Input: Jump */
	void Jump();

	/** Input: Stop jumping */
	void StopJumping();

	/** Input: Start sprint */
	void OnSprintStart();

	/** Input: Stop sprint */
	void OnSprintStop();

	/** Tick survival stat decay */
	void TickSurvivalStats(float DeltaTime);

	/** Check for critical survival conditions */
	void CheckCriticalConditions();
};
