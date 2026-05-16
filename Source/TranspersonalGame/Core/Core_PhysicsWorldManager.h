#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "SharedTypes.h"
#include "Core_PhysicsWorldManager.generated.h"

/**
 * Core_PhysicsWorldManager - Manages global physics settings and world-scale physics simulation
 * Handles gravity variations, physics material assignments, and large-scale destruction events
 * Optimized for prehistoric world with varying terrain and massive dinosaur interactions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACore_PhysicsWorldManager : public AActor
{
    GENERATED_BODY()

public:
    ACore_PhysicsWorldManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === WORLD PHYSICS SETTINGS ===
    
    /** Base gravity for the prehistoric world */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Physics")
    float BaseGravityZ = -980.0f;
    
    /** Current gravity multiplier (for special events like meteor impacts) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Physics")
    float GravityMultiplier = 1.0f;
    
    /** Maximum physics simulation distance from player */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxPhysicsDistance = 50000.0f;
    
    /** Number of physics bodies being actively simulated */
    UPROPERTY(BlueprintReadOnly, Category = "Debug")
    int32 ActivePhysicsBodies = 0;
    
    // === PHYSICS MATERIALS MANAGEMENT ===
    
    /** Default physics material for terrain */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UPhysicalMaterial* TerrainPhysicsMaterial;
    
    /** Physics material for dinosaur bones/scales */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UPhysicalMaterial* DinosaurPhysicsMaterial;
    
    /** Physics material for vegetation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UPhysicalMaterial* VegetationPhysicsMaterial;
    
    /** Physics material for rocks and minerals */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UPhysicalMaterial* RockPhysicsMaterial;
    
    // === LARGE SCALE PHYSICS EVENTS ===
    
    /** Track major destruction events for performance management */
    UPROPERTY(BlueprintReadOnly, Category = "Events")
    TArray<FVector> ActiveDestructionZones;
    
    /** Maximum number of simultaneous destruction zones */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDestructionZones = 5;
    
    // === PHYSICS OPTIMIZATION ===
    
    /** Distance-based LOD for physics simulation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    TArray<float> PhysicsLODDistances = {5000.0f, 15000.0f, 30000.0f};
    
    /** Physics update frequency per LOD level */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    TArray<float> PhysicsUpdateRates = {60.0f, 30.0f, 10.0f};

public:
    // === PHYSICS WORLD MANAGEMENT ===
    
    /** Initialize world physics settings for prehistoric environment */
    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void InitializeWorldPhysics();
    
    /** Apply physics material to actor based on surface type */
    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void ApplyPhysicsMaterialToActor(AActor* Actor, ECore_SurfaceType SurfaceType);
    
    /** Register a new destruction zone for performance tracking */
    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void RegisterDestructionZone(FVector Location, float Radius);
    
    /** Clean up completed destruction zones */
    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void CleanupDestructionZones();
    
    /** Adjust gravity for special events (meteor impact, volcanic activity) */
    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void SetGravityMultiplier(float NewMultiplier, float Duration = 0.0f);
    
    /** Get physics LOD level based on distance from player */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetPhysicsLODLevel(float DistanceFromPlayer) const;
    
    /** Enable/disable physics simulation for actor based on distance */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateActorPhysicsLOD(AActor* Actor, float DistanceFromPlayer);
    
    // === PHYSICS QUERIES ===
    
    /** Get all physics bodies within radius */
    UFUNCTION(BlueprintCallable, Category = "Physics Queries")
    TArray<UPrimitiveComponent*> GetPhysicsBodiesInRadius(FVector Center, float Radius);
    
    /** Check if location is in active destruction zone */
    UFUNCTION(BlueprintCallable, Category = "Physics Queries")
    bool IsLocationInDestructionZone(FVector Location) const;
    
    /** Get current physics simulation load (0.0 to 1.0) */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetPhysicsSimulationLoad() const;

private:
    // === INTERNAL MANAGEMENT ===
    
    /** Timer for gravity multiplier reset */
    FTimerHandle GravityResetTimer;
    
    /** Cache of actors and their physics LOD levels */
    UPROPERTY()
    TMap<AActor*, int32> ActorPhysicsLODCache;
    
    /** Performance tracking */
    float LastPhysicsUpdateTime = 0.0f;
    int32 PhysicsUpdatesThisSecond = 0;
    
    /** Internal physics material assignment */
    void AssignPhysicsMaterialToComponent(UPrimitiveComponent* Component, UPhysicalMaterial* Material);
    
    /** Reset gravity to base value */
    void ResetGravityMultiplier();
    
    /** Update physics performance metrics */
    void UpdatePhysicsMetrics();
};