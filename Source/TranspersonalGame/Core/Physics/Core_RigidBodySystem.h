#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Core_RigidBodySystem.generated.h"

class UStaticMeshComponent;
class UPrimitiveComponent;

/**
 * Rigid body data for physics simulation
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_RigidBodyData
{
	GENERATED_BODY()

	/** Mass of the rigid body in kg */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rigid Body")
	float Mass;
	
	/** Linear velocity in cm/s */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rigid Body")
	FVector LinearVelocity;
	
	/** Angular velocity in degrees/s */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rigid Body")
	FVector AngularVelocity;
	
	/** Center of mass offset from actor origin */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rigid Body")
	FVector CenterOfMass;
	
	/** Linear damping coefficient */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rigid Body")
	float LinearDamping;
	
	/** Angular damping coefficient */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rigid Body")
	float AngularDamping;
	
	/** Is the body currently sleeping (not moving) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rigid Body")
	bool bIsSleeping;

	FCore_RigidBodyData()
	{
		Mass = 100.0f;
		LinearVelocity = FVector::ZeroVector;
		AngularVelocity = FVector::ZeroVector;
		CenterOfMass = FVector::ZeroVector;
		LinearDamping = 0.1f;
		AngularDamping = 0.1f;
		bIsSleeping = false;
	}
};

/**
 * Force application data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_ForceApplication
{
	GENERATED_BODY()

	/** Force vector in Newtons */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force")
	FVector Force;
	
	/** Location to apply force (world space) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force")
	FVector Location;
	
	/** Duration of force application in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force")
	float Duration;
	
	/** Remaining time for force application */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force")
	float RemainingTime;
	
	/** Should force be applied as impulse (single frame) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force")
	bool bIsImpulse;

	FCore_ForceApplication()
	{
		Force = FVector::ZeroVector;
		Location = FVector::ZeroVector;
		Duration = 0.0f;
		RemainingTime = 0.0f;
		bIsImpulse = false;
	}
};

/**
 * Core_RigidBodySystem - Advanced rigid body physics simulation
 * 
 * Provides comprehensive rigid body physics for objects in the prehistoric world.
 * Handles realistic physics simulation for rocks, logs, tools, and other objects
 * that players can interact with, throw, or use as building materials.
 * 
 * Features:
 * - Mass and inertia calculation
 * - Force and impulse application
 * - Collision response and material properties
 * - Sleep/wake state management
 * - Custom physics materials for different object types
 * - Integration with UE5 physics system
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_RigidBodySystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UCore_RigidBodySystem();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// === FORCE APPLICATION ===
	
	/**
	 * Apply force to the rigid body
	 * @param Force - Force vector in Newtons
	 * @param Location - World location to apply force (optional, uses center of mass if zero)
	 * @param bVelChange - Apply as velocity change instead of force
	 */
	UFUNCTION(BlueprintCallable, Category = "Rigid Body Physics")
	void ApplyForce(FVector Force, FVector Location = FVector::ZeroVector, bool bVelChange = false);
	
	/**
	 * Apply impulse to the rigid body (single frame force)
	 * @param Impulse - Impulse vector in Newton-seconds
	 * @param Location - World location to apply impulse
	 */
	UFUNCTION(BlueprintCallable, Category = "Rigid Body Physics")
	void ApplyImpulse(FVector Impulse, FVector Location = FVector::ZeroVector);
	
	/**
	 * Apply torque to the rigid body
	 * @param Torque - Torque vector in Newton-meters
	 * @param bVelChange - Apply as angular velocity change
	 */
	UFUNCTION(BlueprintCallable, Category = "Rigid Body Physics")
	void ApplyTorque(FVector Torque, bool bVelChange = false);
	
	/**
	 * Apply angular impulse to the rigid body
	 * @param AngularImpulse - Angular impulse vector
	 */
	UFUNCTION(BlueprintCallable, Category = "Rigid Body Physics")
	void ApplyAngularImpulse(FVector AngularImpulse);

	// === PHYSICS PROPERTIES ===
	
	/**
	 * Set mass of the rigid body
	 * @param NewMass - New mass in kg
	 */
	UFUNCTION(BlueprintCallable, Category = "Rigid Body Physics")
	void SetMass(float NewMass);
	
	/**
	 * Get current mass of the rigid body
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Rigid Body Physics")
	float GetMass() const;
	
	/**
	 * Set center of mass offset
	 * @param Offset - Offset from actor origin
	 */
	UFUNCTION(BlueprintCallable, Category = "Rigid Body Physics")
	void SetCenterOfMass(FVector Offset);
	
	/**
	 * Get center of mass in world coordinates
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Rigid Body Physics")
	FVector GetCenterOfMassWorldLocation() const;

	// === VELOCITY AND MOTION ===
	
	/**
	 * Set linear velocity of the rigid body
	 * @param Velocity - New velocity in cm/s
	 */
	UFUNCTION(BlueprintCallable, Category = "Rigid Body Physics")
	void SetLinearVelocity(FVector Velocity);
	
	/**
	 * Get current linear velocity
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Rigid Body Physics")
	FVector GetLinearVelocity() const;
	
	/**
	 * Set angular velocity of the rigid body
	 * @param AngularVelocity - New angular velocity in degrees/s
	 */
	UFUNCTION(BlueprintCallable, Category = "Rigid Body Physics")
	void SetAngularVelocity(FVector AngularVelocity);
	
	/**
	 * Get current angular velocity
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Rigid Body Physics")
	FVector GetAngularVelocity() const;

	// === SLEEP/WAKE MANAGEMENT ===
	
	/**
	 * Put rigid body to sleep (stop simulation)
	 */
	UFUNCTION(BlueprintCallable, Category = "Rigid Body Physics")
	void PutToSleep();
	
	/**
	 * Wake up rigid body (resume simulation)
	 */
	UFUNCTION(BlueprintCallable, Category = "Rigid Body Physics")
	void WakeUp();
	
	/**
	 * Check if rigid body is sleeping
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Rigid Body Physics")
	bool IsSleeping() const;

	// === MATERIAL PROPERTIES ===
	
	/**
	 * Set physics material for the rigid body
	 * @param PhysicsMaterial - Physics material to apply
	 */
	UFUNCTION(BlueprintCallable, Category = "Rigid Body Physics")
	void SetPhysicsMaterial(class UPhysicalMaterial* PhysicsMaterial);
	
	/**
	 * Set linear and angular damping
	 * @param LinearDamp - Linear damping coefficient
	 * @param AngularDamp - Angular damping coefficient
	 */
	UFUNCTION(BlueprintCallable, Category = "Rigid Body Physics")
	void SetDamping(float LinearDamp, float AngularDamp);

	// === COLLISION SETTINGS ===
	
	/**
	 * Enable or disable collision for the rigid body
	 * @param bEnabled - Should collision be enabled
	 */
	UFUNCTION(BlueprintCallable, Category = "Rigid Body Physics")
	void SetCollisionEnabled(bool bEnabled);
	
	/**
	 * Set collision response to specific channel
	 * @param Channel - Collision channel
	 * @param Response - Response type
	 */
	UFUNCTION(BlueprintCallable, Category = "Rigid Body Physics")
	void SetCollisionResponseToChannel(ECollisionChannel Channel, ECollisionResponse Response);

	// === PHYSICS CONSTRAINTS ===
	
	/**
	 * Lock specific movement axes
	 * @param bLockX - Lock X axis movement
	 * @param bLockY - Lock Y axis movement  
	 * @param bLockZ - Lock Z axis movement
	 */
	UFUNCTION(BlueprintCallable, Category = "Rigid Body Physics")
	void LockLinearAxes(bool bLockX, bool bLockY, bool bLockZ);
	
	/**
	 * Lock specific rotation axes
	 * @param bLockX - Lock X axis rotation
	 * @param bLockY - Lock Y axis rotation
	 * @param bLockZ - Lock Z axis rotation
	 */
	UFUNCTION(BlueprintCallable, Category = "Rigid Body Physics")
	void LockAngularAxes(bool bLockX, bool bLockY, bool bLockZ);

	// === PHYSICS DATA ACCESS ===
	
	/**
	 * Get complete rigid body data
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Rigid Body Physics")
	FCore_RigidBodyData GetRigidBodyData() const;
	
	/**
	 * Set rigid body data
	 * @param NewData - New rigid body data to apply
	 */
	UFUNCTION(BlueprintCallable, Category = "Rigid Body Physics")
	void SetRigidBodyData(const FCore_RigidBodyData& NewData);

	// === PHYSICS SETTINGS ===
	
	/** Enable physics simulation on this rigid body */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
	bool bSimulatePhysics;
	
	/** Enable gravity for this rigid body */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
	bool bEnableGravity;
	
	/** Auto-sleep when velocity is low */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
	bool bAutoSleep;
	
	/** Velocity threshold for auto-sleep */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (ClampMin = "0.1", ClampMax = "100.0"))
	float SleepThreshold;
	
	/** Time below threshold before sleeping */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float SleepTime;

protected:
	// === INTERNAL PROCESSING ===
	
	/**
	 * Initialize rigid body physics
	 */
	void InitializeRigidBody();
	
	/**
	 * Update physics simulation
	 */
	void UpdatePhysicsSimulation(float DeltaTime);
	
	/**
	 * Process pending force applications
	 */
	void ProcessPendingForces(float DeltaTime);
	
	/**
	 * Update sleep state
	 */
	void UpdateSleepState(float DeltaTime);
	
	/**
	 * Calculate inertia tensor from mesh
	 */
	void CalculateInertiaTensor();
	
	/**
	 * Apply physics material properties
	 */
	void ApplyPhysicsMaterial();

private:
	// === CACHED REFERENCES ===
	
	/** Primary mesh component for physics */
	UPROPERTY()
	UPrimitiveComponent* PhysicsComponent;
	
	/** Current rigid body data */
	UPROPERTY()
	FCore_RigidBodyData RigidBodyData;
	
	/** Pending force applications */
	UPROPERTY()
	TArray<FCore_ForceApplication> PendingForces;
	
	/** Time spent below sleep threshold */
	float TimeAtSleepThreshold;
	
	/** Last frame velocity for sleep detection */
	FVector LastFrameVelocity;
	
	/** Last frame angular velocity for sleep detection */
	FVector LastFrameAngularVelocity;
};