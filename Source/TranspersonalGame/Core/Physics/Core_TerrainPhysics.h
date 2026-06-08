#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Materials/MaterialInterface.h"
#include "Materials/PhysicalMaterial.h"
#include "Core/SharedTypes.h"
#include "Core_TerrainPhysics.generated.h"

/**
 * Core_TerrainPhysics - Advanced terrain physics system for prehistoric survival gameplay
 * 
 * Manages dynamic terrain physics properties including:
 * - Surface material detection and physics response
 * - Terrain deformation and erosion simulation
 * - Environmental physics effects (mud, sand, rock stability)
 * - Biome-specific physics parameters
 * - Performance optimization for large-scale terrain
 * 
 * Integrates with PCGWorldGenerator for procedural terrain physics setup
 * Provides realistic ground interaction for dinosaurs and player character
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TerrainSurfaceData
{
    GENERATED_BODY()

    /** Surface material type affecting physics behavior */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    ESurfaceMaterial SurfaceType = ESurfaceMaterial::Rock;

    /** Friction coefficient for this surface (0.0 = ice, 1.0 = rubber) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float FrictionCoefficient = 0.7f;

    /** Restitution (bounciness) of the surface (0.0 = no bounce, 1.0 = perfect bounce) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Restitution = 0.1f;

    /** Density of the surface material (kg/m³) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (ClampMin = "100.0", ClampMax = "10000.0"))
    float Density = 2000.0f;

    /** How much this surface can deform under pressure (0.0 = rigid, 1.0 = soft) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DeformationResistance = 0.8f;

    /** Sound effect to play when objects impact this surface */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    TSoftObjectPtr<class USoundBase> ImpactSound;

    /** Particle effect to spawn on impact */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    TSoftObjectPtr<class UNiagaraSystem> ImpactParticles;

    FCore_TerrainSurfaceData()
    {
        SurfaceType = ESurfaceMaterial::Rock;
        FrictionCoefficient = 0.7f;
        Restitution = 0.1f;
        Density = 2000.0f;
        DeformationResistance = 0.8f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TerrainPhysicsZone
{
    GENERATED_BODY()

    /** Zone identifier for this physics region */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Zone")
    FString ZoneID;

    /** Biome type affecting physics parameters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Zone")
    EBiomeType BiomeType = EBiomeType::Grassland;

    /** Center location of this physics zone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Zone")
    FVector ZoneCenter = FVector::ZeroVector;

    /** Radius of influence for this zone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Zone", meta = (ClampMin = "100.0"))
    float ZoneRadius = 5000.0f;

    /** Gravity modifier for this zone (1.0 = normal gravity) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Zone", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float GravityMultiplier = 1.0f;

    /** Wind force affecting objects in this zone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Zone")
    FVector WindForce = FVector::ZeroVector;

    /** Temperature affecting material properties */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Zone")
    float Temperature = 20.0f;

    /** Humidity level affecting friction and deformation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Zone", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Humidity = 0.5f;

    /** Surface materials present in this zone */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Zone")
    TArray<FCore_TerrainSurfaceData> SurfaceMaterials;

    FCore_TerrainPhysicsZone()
    {
        ZoneID = TEXT("DefaultZone");
        BiomeType = EBiomeType::Grassland;
        ZoneCenter = FVector::ZeroVector;
        ZoneRadius = 5000.0f;
        GravityMultiplier = 1.0f;
        WindForce = FVector::ZeroVector;
        Temperature = 20.0f;
        Humidity = 0.5f;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_TerrainPhysics : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_TerrainPhysics();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === TERRAIN PHYSICS MANAGEMENT ===

    /** Initialize terrain physics system for the current world */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void InitializeTerrainPhysics();

    /** Register a physics zone with specific parameters */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void RegisterPhysicsZone(const FCore_TerrainPhysicsZone& PhysicsZone);

    /** Get surface data at a specific world location */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    FCore_TerrainSurfaceData GetSurfaceDataAtLocation(const FVector& WorldLocation);

    /** Apply environmental forces to an actor based on terrain */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyEnvironmentalForces(AActor* TargetActor, float DeltaTime);

    /** Check if location is suitable for building/placing objects */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool IsLocationStable(const FVector& WorldLocation, float RequiredStability = 0.7f);

    // === TERRAIN DEFORMATION ===

    /** Deform terrain at location with specified parameters */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void DeformTerrainAtLocation(const FVector& WorldLocation, float DeformationRadius, float DeformationStrength, bool bIsAdditive = false);

    /** Create crater from explosion or impact */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void CreateImpactCrater(const FVector& ImpactLocation, float CraterRadius, float CraterDepth);

    /** Simulate erosion effects over time */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void SimulateErosion(float DeltaTime);

    // === PHYSICS OPTIMIZATION ===

    /** Update physics LOD based on distance from players */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UpdatePhysicsLOD();

    /** Enable/disable physics simulation for distant terrain */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void SetPhysicsSimulationEnabled(bool bEnabled, float DistanceThreshold = 10000.0f);

    /** Get current physics performance metrics */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    FString GetPhysicsPerformanceStats();

protected:
    // === CORE PROPERTIES ===

    /** All registered physics zones in the world */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain Physics")
    TArray<FCore_TerrainPhysicsZone> PhysicsZones;

    /** Default surface data for unknown terrain types */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    FCore_TerrainSurfaceData DefaultSurfaceData;

    /** Maximum distance for physics simulation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (ClampMin = "1000.0"))
    float MaxPhysicsDistance = 15000.0f;

    /** Update frequency for physics zones (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (ClampMin = "0.1"))
    float PhysicsUpdateInterval = 1.0f;

    /** Enable terrain deformation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bEnableTerrainDeformation = true;

    /** Enable erosion simulation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bEnableErosionSimulation = false;

    /** Performance optimization level (0=disabled, 3=maximum) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics", meta = (ClampMin = "0", ClampMax = "3"))
    int32 OptimizationLevel = 1;

private:
    // === INTERNAL SYSTEMS ===

    /** Timer for physics zone updates */
    float PhysicsUpdateTimer = 0.0f;

    /** Cached landscape reference */
    UPROPERTY()
    TWeakObjectPtr<ALandscape> CachedLandscape;

    /** Cached player references for distance calculations */
    UPROPERTY()
    TArray<TWeakObjectPtr<APawn>> CachedPlayers;

    /** Performance tracking */
    int32 ActivePhysicsZones = 0;
    int32 DeformationOperationsThisFrame = 0;
    float LastPerformanceCheck = 0.0f;

    // === HELPER FUNCTIONS ===

    /** Find the landscape actor in the current world */
    ALandscape* FindLandscapeActor();

    /** Get height and normal at world location */
    bool GetTerrainHeightAndNormal(const FVector& WorldLocation, float& OutHeight, FVector& OutNormal);

    /** Calculate surface material based on slope and biome */
    ESurfaceMaterial CalculateSurfaceMaterial(const FVector& WorldLocation, const FVector& SurfaceNormal);

    /** Apply biome-specific physics modifications */
    void ApplyBiomePhysicsModifications(FCore_TerrainSurfaceData& SurfaceData, EBiomeType BiomeType);

    /** Update cached player references */
    void UpdateCachedPlayers();

    /** Calculate distance-based LOD level */
    int32 CalculatePhysicsLOD(const FVector& Location);
};