#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Landscape/Landscape.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "Core_TerrainPhysicsSystem.generated.h"

/**
 * Core_TerrainPhysicsSystem - Advanced terrain physics management for prehistoric world
 * 
 * Handles realistic terrain physics including:
 * - Dynamic surface material properties based on biome type
 * - Footprint and track generation on soft surfaces
 * - Erosion simulation for realistic weathering
 * - Slope stability and landslide mechanics
 * - Mud and quicksand physics in swamp biomes
 * - Rock hardness variation for climbing and destruction
 * 
 * Optimized for large-scale prehistoric landscapes with performance LOD.
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_TerrainPhysicsSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_TerrainPhysicsSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === TERRAIN MATERIAL PROPERTIES ===
    
    /** Surface friction coefficient for different terrain types */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    TMap<ECore_BiomeType, float> BiomeFrictionValues;
    
    /** Surface hardness affecting footprint depth */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    TMap<ECore_BiomeType, float> BiomeHardnessValues;
    
    /** Moisture level affecting mud physics */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    TMap<ECore_BiomeType, float> BiomeMoistureValues;
    
    // === FOOTPRINT SYSTEM ===
    
    /** Enable dynamic footprint generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprints")
    bool bEnableFootprints;
    
    /** Maximum number of active footprints */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprints")
    int32 MaxFootprints;
    
    /** Footprint fade time in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprints")
    float FootprintFadeTime;
    
    /** Minimum weight required to leave footprints */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footprints")
    float MinFootprintWeight;
    
    // === EROSION SYSTEM ===
    
    /** Enable terrain erosion simulation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    bool bEnableErosion;
    
    /** Rain intensity affecting erosion rate */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float RainIntensity;
    
    /** Wind strength for sand/dust erosion */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float WindStrength;
    
    /** Erosion update frequency (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erosion")
    float ErosionUpdateFrequency;
    
    // === SLOPE PHYSICS ===
    
    /** Maximum stable slope angle in degrees */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope Physics")
    float MaxStableSlope;
    
    /** Landslide trigger threshold */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope Physics")
    float LandslideThreshold;
    
    /** Enable dynamic slope stability checks */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slope Physics")
    bool bEnableSlopePhysics;
    
    // === MUD AND QUICKSAND ===
    
    /** Mud viscosity in swamp areas */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mud Physics")
    float MudViscosity;
    
    /** Quicksand sink rate */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mud Physics")
    float QuicksandSinkRate;
    
    /** Enable mud physics in swamp biomes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mud Physics")
    bool bEnableMudPhysics;
    
    // === PERFORMANCE SETTINGS ===
    
    /** Physics LOD distance thresholds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    TArray<float> PhysicsLODDistances;
    
    /** Update frequency for distant terrain physics */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float DistantPhysicsUpdateRate;
    
    /** Maximum terrain physics calculations per frame */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxPhysicsCalculationsPerFrame;

    // === PUBLIC INTERFACE ===
    
    /** Apply terrain physics to a specific location */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyTerrainPhysicsAtLocation(const FVector& Location, float Radius, float Force);
    
    /** Get terrain properties at specific location */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    FCore_TerrainProperties GetTerrainPropertiesAtLocation(const FVector& Location);
    
    /** Create footprint at location */
    UFUNCTION(BlueprintCallable, Category = "Footprints")
    void CreateFootprint(const FVector& Location, float Weight, float Size, ECore_CreatureType CreatureType);
    
    /** Trigger erosion in area */
    UFUNCTION(BlueprintCallable, Category = "Erosion")
    void TriggerErosionInArea(const FVector& Center, float Radius, float Intensity);
    
    /** Check if location is stable for building */
    UFUNCTION(BlueprintCallable, Category = "Slope Physics")
    bool IsLocationStableForBuilding(const FVector& Location, float StructureWeight);
    
    /** Get mud depth at location */
    UFUNCTION(BlueprintCallable, Category = "Mud Physics")
    float GetMudDepthAtLocation(const FVector& Location);

private:
    // === INTERNAL STATE ===
    
    /** Current active footprints */
    TArray<FCore_FootprintData> ActiveFootprints;
    
    /** Erosion calculation timer */
    float ErosionTimer;
    
    /** Physics calculation counter for performance */
    int32 CurrentFrameCalculations;
    
    /** Cached landscape reference */
    UPROPERTY()
    ALandscape* CachedLandscape;
    
    /** Cached biome manager reference */
    UPROPERTY()
    class UBiomeManager* BiomeManager;
    
    // === INTERNAL METHODS ===
    
    /** Initialize terrain physics system */
    void InitializeTerrainPhysics();
    
    /** Update footprint system */
    void UpdateFootprints(float DeltaTime);
    
    /** Process erosion calculations */
    void ProcessErosion(float DeltaTime);
    
    /** Check slope stability */
    void CheckSlopeStability();
    
    /** Update mud physics */
    void UpdateMudPhysics(float DeltaTime);
    
    /** Get biome type at location */
    ECore_BiomeType GetBiomeTypeAtLocation(const FVector& Location);
    
    /** Calculate terrain normal at location */
    FVector GetTerrainNormalAtLocation(const FVector& Location);
    
    /** Get terrain height at location */
    float GetTerrainHeightAtLocation(const FVector& Location);
    
    /** Apply performance LOD based on distance */
    bool ShouldUpdatePhysicsAtDistance(float Distance);
    
    /** Clean up old footprints */
    void CleanupOldFootprints();
    
    /** Calculate erosion factor */
    float CalculateErosionFactor(const FVector& Location, float RainAmount, float WindAmount);
    
    /** Apply terrain deformation */
    void ApplyTerrainDeformation(const FVector& Location, float Intensity, float Radius);
};