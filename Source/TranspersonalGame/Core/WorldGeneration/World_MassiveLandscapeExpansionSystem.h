#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "LandscapeComponent.h"
#include "LandscapeInfo.h"
#include "LandscapeDataAccess.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "World_MassiveLandscapeExpansionSystem.generated.h"

/**
 * Massive Landscape Expansion System for 10km x 10km Cretaceous World
 * Fulfills purchase criteria requirement for large-scale terrain generation
 * Integrates with performance monitoring from Agent #4's weather physics system
 */
UCLASS(BlueprintType, Blueprintable, Category = "World Generation")
class TRANSPERSONALGAME_API AWorld_MassiveLandscapeExpansionSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_MassiveLandscapeExpansionSystem();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // === LANDSCAPE EXPANSION CORE ===
    
    /** Create or expand landscape to 10km x 10km dimensions */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Landscape Expansion")
    void CreateMassiveLandscape();
    
    /** Expand existing landscape to target dimensions */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Landscape Expansion")
    void ExpandExistingLandscape(ALandscape* TargetLandscape);
    
    /** Generate 5 distinct biome zones across the massive landscape */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome Generation")
    void GenerateBiomeZones();
    
    /** Apply height variation and terrain features */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Terrain Generation")
    void ApplyTerrainFeatures();

    // === BIOME ZONE MANAGEMENT ===
    
    /** Create Swamp biome in Southwest quadrant */
    UFUNCTION(BlueprintCallable, Category = "Biome Creation")
    void CreateSwampBiome(FVector ZoneCenter, float ZoneRadius);
    
    /** Create Forest biome in Northwest quadrant */
    UFUNCTION(BlueprintCallable, Category = "Biome Creation")
    void CreateForestBiome(FVector ZoneCenter, float ZoneRadius);
    
    /** Create Savanna biome in center area */
    UFUNCTION(BlueprintCallable, Category = "Biome Creation")
    void CreateSavannaBiome(FVector ZoneCenter, float ZoneRadius);
    
    /** Create Desert biome in East quadrant */
    UFUNCTION(BlueprintCallable, Category = "Biome Creation")
    void CreateDesertBiome(FVector ZoneCenter, float ZoneRadius);
    
    /** Create Mountain biome in Northeast quadrant */
    UFUNCTION(BlueprintCallable, Category = "Biome Creation")
    void CreateMountainBiome(FVector ZoneCenter, float ZoneRadius);

    // === PERFORMANCE INTEGRATION ===
    
    /** Check performance impact of landscape expansion */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void MonitorLandscapePerformance();
    
    /** Apply LOD optimization for massive terrain */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyLandscapeLOD();

    // === LANDSCAPE PROPERTIES ===
    
    /** Target landscape dimensions in Unreal units (10km = 1,000,000 units) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Settings")
    FVector2D TargetLandscapeDimensions = FVector2D(1000000.0f, 1000000.0f);
    
    /** Landscape height scale for terrain variation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Settings")
    float LandscapeHeightScale = 500.0f;
    
    /** Number of landscape components per axis */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Settings")
    int32 ComponentsPerAxis = 64;
    
    /** Landscape material for terrain rendering */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Settings")
    TSoftObjectPtr<UMaterialInterface> LandscapeMaterial;

    // === BIOME ZONE DEFINITIONS ===
    
    /** Swamp biome zone center and radius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zones")
    FVector SwampZoneCenter = FVector(-250000.0f, -250000.0f, 0.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zones")
    float SwampZoneRadius = 200000.0f;
    
    /** Forest biome zone center and radius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zones")
    FVector ForestZoneCenter = FVector(-250000.0f, 250000.0f, 0.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zones")
    float ForestZoneRadius = 200000.0f;
    
    /** Savanna biome zone center and radius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zones")
    FVector SavannaZoneCenter = FVector(0.0f, 0.0f, 0.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zones")
    float SavannaZoneRadius = 150000.0f;
    
    /** Desert biome zone center and radius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zones")
    FVector DesertZoneCenter = FVector(350000.0f, 0.0f, 0.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zones")
    float DesertZoneRadius = 180000.0f;
    
    /** Mountain biome zone center and radius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zones")
    FVector MountainZoneCenter = FVector(250000.0f, 250000.0f, 0.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zones")
    float MountainZoneRadius = 200000.0f;

    // === PERFORMANCE MONITORING ===
    
    /** Performance threshold for landscape rendering (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceThresholdMs = 16.67f; // 60fps target
    
    /** Current landscape performance metrics */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentRenderTimeMs = 0.0f;
    
    /** Enable automatic LOD adjustment based on performance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bAutoLODAdjustment = true;
    
    /** Current landscape LOD level (0-4) */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CurrentLODLevel = 0;

    // === TERRAIN GENERATION SETTINGS ===
    
    /** Noise scale for terrain height generation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    float TerrainNoiseScale = 0.001f;
    
    /** Terrain height variation amplitude */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    float TerrainHeightAmplitude = 300.0f;
    
    /** Number of octaves for terrain noise */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    int32 TerrainNoiseOctaves = 4;
    
    /** Terrain feature density (0.0 - 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation")
    float TerrainFeatureDensity = 0.3f;

protected:
    /** Reference to the main landscape actor */
    UPROPERTY()
    ALandscape* MainLandscape = nullptr;
    
    /** Array of biome zone actors */
    UPROPERTY()
    TArray<AActor*> BiomeZoneActors;
    
    /** Performance monitoring timer */
    FTimerHandle PerformanceMonitorTimer;
    
    /** Landscape expansion state */
    bool bLandscapeExpanded = false;
    
    /** Biome zones generated state */
    bool bBiomeZonesGenerated = false;

private:
    /** Internal helper for landscape component creation */
    void CreateLandscapeComponents(ALandscape* Landscape, int32 ComponentCountX, int32 ComponentCountY);
    
    /** Internal helper for height data generation */
    void GenerateHeightData(TArray<uint16>& HeightData, int32 SizeX, int32 SizeY);
    
    /** Internal helper for biome transition blending */
    void BlendBiomeTransitions();
    
    /** Internal performance monitoring function */
    void UpdatePerformanceMetrics();
};