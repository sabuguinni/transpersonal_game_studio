#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "Core_PhysicsWorldSimulator.generated.h"

/**
 * Core Physics World Simulator
 * Manages real-time physics simulation for the prehistoric world
 * Handles dynamic terrain interaction, object physics, and environmental forces
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsSimulationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    float GravityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation") 
    float AirDensity = 1.225f; // kg/m³ at sea level

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    float WindStrength = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    bool bEnableAdvancedPhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    float SimulationTimeScale = 1.0f;

    FCore_PhysicsSimulationSettings()
    {
        GravityScale = 1.0f;
        AirDensity = 1.225f;
        WindStrength = 0.0f;
        WindDirection = FVector(1.0f, 0.0f, 0.0f);
        bEnableAdvancedPhysics = true;
        SimulationTimeScale = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsObjectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Mass = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Density = 1000.0f; // kg/m³

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Friction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Restitution = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bIsDestructible = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float DestructionThreshold = 1000.0f; // Force required to destroy

    FCore_PhysicsObjectData()
    {
        Mass = 1.0f;
        Density = 1000.0f;
        Friction = 0.7f;
        Restitution = 0.3f;
        bIsDestructible = false;
        DestructionThreshold = 1000.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsWorldSimulator : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsWorldSimulator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core simulation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Simulation")
    FCore_PhysicsSimulationSettings SimulationSettings;

    // Physics object registry
    UPROPERTY(BlueprintReadOnly, Category = "Physics Simulation")
    TMap<AActor*, FCore_PhysicsObjectData> PhysicsObjects;

    // Environmental forces
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Forces")
    bool bApplyWindForces = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Forces")
    bool bApplyGravityVariation = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Forces")
    float GravityVariationStrength = 0.1f;

    // Simulation control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Control")
    bool bIsSimulationActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Control")
    float MaxSimulationDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation Control")
    int32 MaxSimulatedObjects = 1000;

    // Physics simulation methods
    UFUNCTION(BlueprintCallable, Category = "Physics Simulation")
    void StartSimulation();

    UFUNCTION(BlueprintCallable, Category = "Physics Simulation")
    void StopSimulation();

    UFUNCTION(BlueprintCallable, Category = "Physics Simulation")
    void ResetSimulation();

    UFUNCTION(BlueprintCallable, Category = "Physics Simulation")
    void RegisterPhysicsObject(AActor* Actor, const FCore_PhysicsObjectData& PhysicsData);

    UFUNCTION(BlueprintCallable, Category = "Physics Simulation")
    void UnregisterPhysicsObject(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Simulation")
    void ApplyForceToObject(AActor* Actor, const FVector& Force, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Physics Simulation")
    void ApplyImpulseToObject(AActor* Actor, const FVector& Impulse, const FVector& Location);

    // Environmental effects
    UFUNCTION(BlueprintCallable, Category = "Environmental Forces")
    void SetWindParameters(float Strength, const FVector& Direction);

    UFUNCTION(BlueprintCallable, Category = "Environmental Forces")
    void SetGravityScale(float NewGravityScale);

    UFUNCTION(BlueprintCallable, Category = "Environmental Forces")
    void ApplyEnvironmentalForces(float DeltaTime);

    // Terrain interaction
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool CheckTerrainCollision(const FVector& Location, float Radius, FHitResult& OutHit);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    float GetTerrainHeightAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    FVector GetTerrainNormalAtLocation(const FVector& Location);

    // Simulation optimization
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizeSimulation();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void UpdateSimulationLOD(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    bool ShouldSimulateObject(AActor* Actor) const;

    // Debug and monitoring
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DebugDrawPhysicsObjects();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogSimulationStats();

    UFUNCTION(BlueprintPure, Category = "Debug")
    int32 GetActivePhysicsObjectCount() const;

    UFUNCTION(BlueprintPure, Category = "Debug")
    float GetSimulationPerformanceMetric() const;

private:
    // Internal simulation state
    float AccumulatedTime;
    float LastOptimizationTime;
    int32 SimulationFrameCount;
    
    // Performance tracking
    float AverageFrameTime;
    float PeakFrameTime;
    int32 TotalObjectsProcessed;

    // Internal methods
    void UpdatePhysicsObjects(float DeltaTime);
    void ProcessCollisions(float DeltaTime);
    void ApplyGravityVariations(float DeltaTime);
    void UpdatePerformanceMetrics(float DeltaTime);
    bool IsWithinSimulationRange(AActor* Actor) const;
    void CleanupInvalidObjects();
};