#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Physics/PhysicsFiltering.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "SharedTypes.h"
#include "PhysicsWorldSimulator.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPhysicsSimulationStep, float, DeltaTime, int32, StepCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPhysicsCollisionEvent, AActor*, ActorA, AActor*, ActorB, float, ImpactForce);

/**
 * Advanced physics world simulator that manages complex physics scenarios
 * for the prehistoric survival environment. Handles large-scale physics
 * simulations including dinosaur herds, environmental destruction, and
 * realistic terrain deformation.
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPhysicsWorldSimulator : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysicsWorldSimulator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === SIMULATION CONTROL ===
    
    /** Start the physics world simulation */
    UFUNCTION(BlueprintCallable, Category = "Physics Simulation")
    void StartSimulation();
    
    /** Pause the physics world simulation */
    UFUNCTION(BlueprintCallable, Category = "Physics Simulation")
    void PauseSimulation();
    
    /** Reset the physics world simulation */
    UFUNCTION(BlueprintCallable, Category = "Physics Simulation")
    void ResetSimulation();
    
    /** Set simulation time scale */
    UFUNCTION(BlueprintCallable, Category = "Physics Simulation")
    void SetSimulationTimeScale(float TimeScale);

    // === LARGE-SCALE PHYSICS ===
    
    /** Simulate dinosaur herd physics with collision avoidance */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    void SimulateDinosaurHerdPhysics(const TArray<AActor*>& DinosaurActors, float HerdRadius, float AvoidanceForce);
    
    /** Apply environmental forces to all physics objects */
    UFUNCTION(BlueprintCallable, Category = "Environmental Physics")
    void ApplyEnvironmentalForces(FVector WindDirection, float WindStrength, float GravityMultiplier);
    
    /** Simulate earthquake effects on terrain and objects */
    UFUNCTION(BlueprintCallable, Category = "Environmental Physics")
    void SimulateEarthquake(FVector EpicenterLocation, float Magnitude, float Duration);

    // === DESTRUCTION SIMULATION ===
    
    /** Trigger destruction event at location */
    UFUNCTION(BlueprintCallable, Category = "Destruction Physics")
    void TriggerDestructionEvent(FVector Location, float ExplosionForce, float Radius);
    
    /** Simulate tree falling physics */
    UFUNCTION(BlueprintCallable, Category = "Destruction Physics")
    void SimulateTreeFalling(AActor* TreeActor, FVector ImpactDirection, float Force);
    
    /** Simulate rock avalanche */
    UFUNCTION(BlueprintCallable, Category = "Destruction Physics")
    void SimulateRockAvalanche(FVector StartLocation, FVector Direction, int32 RockCount);

    // === PERFORMANCE OPTIMIZATION ===
    
    /** Set physics LOD based on distance from player */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePhysicsLOD(AActor* PlayerActor, float HighDetailRadius, float MediumDetailRadius);
    
    /** Optimize physics simulation for performance */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsSimulation();
    
    /** Get current physics performance metrics */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FCore_PhysicsPerformanceMetrics GetPhysicsPerformanceMetrics() const;

    // === COLLISION MANAGEMENT ===
    
    /** Register collision event callback */
    UFUNCTION(BlueprintCallable, Category = "Collision")
    void RegisterCollisionCallback(AActor* Actor);
    
    /** Unregister collision event callback */
    UFUNCTION(BlueprintCallable, Category = "Collision")
    void UnregisterCollisionCallback(AActor* Actor);
    
    /** Handle collision event */
    UFUNCTION()
    void HandleCollisionEvent(AActor* ActorA, AActor* ActorB, const FVector& ImpactPoint, float ImpactForce);

    // === ADVANCED FEATURES ===
    
    /** Simulate realistic water physics */
    UFUNCTION(BlueprintCallable, Category = "Fluid Physics")
    void SimulateWaterPhysics(FVector WaterSurfaceLocation, float WaterLevel, float FlowRate);
    
    /** Apply realistic creature physics to dinosaur */
    UFUNCTION(BlueprintCallable, Category = "Creature Physics")
    void ApplyCreaturePhysics(AActor* CreatureActor, float Mass, float Strength, ECore_CreatureSize CreatureSize);
    
    /** Simulate realistic terrain deformation */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void DeformTerrain(FVector Location, float Radius, float Depth, ECore_TerrainDeformationType DeformationType);

protected:
    // === CONFIGURATION ===
    
    /** Enable advanced physics simulation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableAdvancedPhysics;
    
    /** Maximum number of physics objects to simulate */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxPhysicsObjects;
    
    /** Physics simulation time step */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float PhysicsTimeStep;
    
    /** Enable performance optimization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnablePerformanceOptimization;

    // === SIMULATION STATE ===
    
    /** Current simulation time scale */
    UPROPERTY(BlueprintReadOnly, Category = "Simulation State")
    float CurrentTimeScale;
    
    /** Is simulation currently running */
    UPROPERTY(BlueprintReadOnly, Category = "Simulation State")
    bool bIsSimulationRunning;
    
    /** Current simulation step count */
    UPROPERTY(BlueprintReadOnly, Category = "Simulation State")
    int32 SimulationStepCount;
    
    /** Active physics objects count */
    UPROPERTY(BlueprintReadOnly, Category = "Simulation State")
    int32 ActivePhysicsObjectsCount;

    // === PERFORMANCE TRACKING ===
    
    /** Physics simulation time per frame */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsSimulationTime;
    
    /** Collision detection time per frame */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CollisionDetectionTime;
    
    /** Memory usage for physics simulation */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsMemoryUsage;

    // === EVENTS ===
    
    /** Called each physics simulation step */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPhysicsSimulationStep OnPhysicsSimulationStep;
    
    /** Called when physics collision occurs */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPhysicsCollisionEvent OnPhysicsCollisionEvent;

private:
    // === INTERNAL STATE ===
    
    /** Registered collision actors */
    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> RegisteredCollisionActors;
    
    /** Physics world reference */
    UPROPERTY()
    TWeakObjectPtr<UWorld> PhysicsWorld;
    
    /** Performance metrics cache */
    mutable FCore_PhysicsPerformanceMetrics CachedMetrics;
    
    /** Last performance update time */
    float LastPerformanceUpdateTime;

    // === INTERNAL METHODS ===
    
    /** Initialize physics world simulation */
    void InitializePhysicsWorld();
    
    /** Update performance metrics */
    void UpdatePerformanceMetrics(float DeltaTime);
    
    /** Apply LOD optimization to physics objects */
    void ApplyPhysicsLODOptimization(const FVector& PlayerLocation);
    
    /** Clean up inactive physics objects */
    void CleanupInactivePhysicsObjects();
    
    /** Validate physics simulation state */
    bool ValidatePhysicsSimulationState() const;
};