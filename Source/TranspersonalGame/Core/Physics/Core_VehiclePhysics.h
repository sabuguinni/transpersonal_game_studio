#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Core_VehiclePhysics.generated.h"

class UStaticMeshComponent;
class APawn;

/**
 * Core_VehiclePhysics - Primitive vehicle physics system for prehistoric transportation
 * 
 * Handles physics simulation for simple vehicles like wooden carts, sleds, and rafts
 * that would be available in a prehistoric survival setting. Includes realistic
 * terrain interaction, suspension simulation, and stability control.
 * 
 * Features:
 * - Primitive vehicle dynamics (acceleration, steering, braking)
 * - Terrain-specific physics (sand, mud, snow, water resistance)
 * - Basic suspension system for rough terrain
 * - Vehicle stability and anti-roll systems
 * - Realistic speed and force limitations for primitive technology
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_VehiclePhysics : public UActorComponent
{
	GENERATED_BODY()

public:
	UCore_VehiclePhysics();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// === VEHICLE CONTROL INTERFACE ===
	
	/**
	 * Apply throttle input to the vehicle
	 * @param ThrottleInput - Input value from -1.0 (reverse) to 1.0 (forward)
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
	void ApplyThrottle(float ThrottleInput);
	
	/**
	 * Apply steering input to the vehicle
	 * @param SteeringInput - Input value from -1.0 (left) to 1.0 (right)
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
	void ApplySteering(float SteeringInput);
	
	/**
	 * Apply braking force to the vehicle
	 * @param BrakeInput - Input value from 0.0 (no brake) to 1.0 (full brake)
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Physics")
	void ApplyBraking(float BrakeInput);

	// === VEHICLE PROPERTIES ===
	
	/** Maximum speed in cm/s (default: 2000 = 20 m/s for primitive vehicles) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
	float MaxSpeed;
	
	/** Acceleration force in N */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics", meta = (ClampMin = "100.0", ClampMax = "2000.0"))
	float Acceleration;
	
	/** Deceleration/braking force in N */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics", meta = (ClampMin = "200.0", ClampMax = "3000.0"))
	float Deceleration;
	
	/** Turn rate in degrees per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics", meta = (ClampMin = "30.0", ClampMax = "180.0"))
	float TurnRate;

	// === STABILITY SETTINGS ===
	
	/** Force applied to keep vehicle stable (prevent flipping) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Stability", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float StabilityForce;
	
	/** Anti-roll force to prevent rolling over */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Stability", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float AntiRollForce;
	
	/** Downforce applied at high speeds for stability */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Stability", meta = (ClampMin = "0.0", ClampMax = "500.0"))
	float DownForce;

	// === TERRAIN INTERACTION ===
	
	/** Base terrain friction coefficient */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Interaction", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float TerrainFriction;
	
	/** Water resistance coefficient */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Interaction", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float WaterResistance;
	
	/** Maximum slope angle the vehicle can climb (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Interaction", meta = (ClampMin = "15.0", ClampMax = "60.0"))
	float SlopeLimit;

	// === STATUS QUERIES ===
	
	/**
	 * Check if vehicle is currently stable (not flipping/rolling)
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Vehicle Physics")
	bool IsVehicleStable() const;
	
	/**
	 * Get current vehicle speed in cm/s
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Vehicle Physics")
	float GetCurrentSpeed() const;
	
	/**
	 * Check if vehicle is in contact with ground
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Vehicle Physics")
	bool IsOnGround() const;
	
	/**
	 * Check if vehicle is in water
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Vehicle Physics")
	bool IsInWater() const;
	
	/**
	 * Get current terrain type under vehicle
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Vehicle Physics")
	ECore_TerrainType GetCurrentTerrainType() const;

protected:
	// === INTERNAL PHYSICS PROCESSING ===
	
	/**
	 * Update vehicle physics state each frame
	 */
	void UpdatePhysicsState(float DeltaTime);
	
	/**
	 * Apply vehicle-specific forces (downforce, stability, etc.)
	 */
	void ApplyVehicleForces(float DeltaTime);
	
	/**
	 * Handle terrain-specific physics interactions
	 */
	void HandleTerrainInteraction(float DeltaTime);
	
	/**
	 * Initialize suspension system
	 */
	void InitializeSuspension();
	
	/**
	 * Update suspension physics
	 */
	void UpdateSuspension(float DeltaTime);
	
	/**
	 * Update ground contact detection
	 */
	void UpdateGroundContact();
	
	/**
	 * Update water contact detection
	 */
	void UpdateWaterContact();
	
	/**
	 * Update current terrain type
	 */
	void UpdateTerrainType();
	
	/**
	 * Get speed modifier based on current terrain
	 */
	float GetTerrainSpeedModifier() const;
	
	/**
	 * Apply stability force to prevent vehicle flipping
	 */
	void ApplyStabilityForce();
	
	/**
	 * Apply anti-roll force
	 */
	void ApplyAntiRollForce();
	
	/**
	 * Apply sand-specific resistance
	 */
	void ApplySandResistance();
	
	/**
	 * Apply mud-specific resistance
	 */
	void ApplyMudResistance();
	
	/**
	 * Apply snow-specific resistance
	 */
	void ApplySnowResistance();
	
	/**
	 * Apply water resistance
	 */
	void ApplyWaterResistance();

private:
	// === CACHED REFERENCES ===
	
	/** Owner pawn reference */
	UPROPERTY()
	APawn* OwnerPawn;
	
	/** Main mesh component for physics */
	UPROPERTY()
	UStaticMeshComponent* MeshComponent;
	
	/** Wheel components (for wheeled vehicles) */
	UPROPERTY()
	TArray<UStaticMeshComponent*> WheelComponents;

	// === PHYSICS STATE ===
	
	/** Current vehicle speed in cm/s */
	float CurrentSpeed;
	
	/** Is vehicle currently on ground */
	bool bIsOnGround;
	
	/** Is vehicle currently in water */
	bool bIsInWater;
	
	/** Current terrain type under vehicle */
	ECore_TerrainType CurrentTerrainType;
	
	/** Suspension points for basic suspension simulation */
	TArray<FVector> SuspensionPoints;
};