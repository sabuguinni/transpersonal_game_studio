#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_CretaceousTerrainExpansion.generated.h"

/**
 * World_CretaceousTerrainExpansion
 * 
 * Creates and manages the massive 10km x 10km Cretaceous terrain required for asset purchase criteria.
 * This system handles the creation of the main landscape/terrain plane that serves as the foundation
 * for all biome placement and environmental systems.
 * 
 * Key Features:
 * - 10km x 10km terrain base (minimum requirement for asset purchase)
 * - Biome zone allocation (5 distinct regions)
 * - Height variation and geological features
 * - Integration with existing biome systems
 * - Performance-optimized terrain streaming
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_CretaceousTerrainExpansion : public AActor
{
    GENERATED_BODY()

public:
    AWorld_CretaceousTerrainExpansion();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core terrain creation and management
    UFUNCTION(BlueprintCallable, Category = "Terrain Expansion")
    void CreateMassiveTerrain();

    UFUNCTION(BlueprintCallable, Category = "Terrain Expansion")
    void ExpandTerrainTo10km();

    UFUNCTION(BlueprintCallable, Category = "Terrain Expansion")
    void SetupBiomeZones();

    UFUNCTION(BlueprintCallable, Category = "Terrain Expansion")
    void CreateHeightVariation();

    UFUNCTION(BlueprintCallable, Category = "Terrain Expansion")
    void OptimizeTerrainPerformance();

    // Biome zone management
    UFUNCTION(BlueprintCallable, Category = "Biome Zones")
    void AllocateSwampZone();

    UFUNCTION(BlueprintCallable, Category = "Biome Zones")
    void AllocateForestZone();

    UFUNCTION(BlueprintCallable, Category = "Biome Zones")
    void AllocateSavannaZone();

    UFUNCTION(BlueprintCallable, Category = "Biome Zones")
    void AllocateDesertZone();

    UFUNCTION(BlueprintCallable, Category = "Biome Zones")
    void AllocateMountainZone();

    // Terrain features
    UFUNCTION(BlueprintCallable, Category = "Terrain Features")
    void CreateRiverSystems();

    UFUNCTION(BlueprintCallable, Category = "Terrain Features")
    void CreateMountainRanges();

    UFUNCTION(BlueprintCallable, Category = "Terrain Features")
    void CreateValleysAndPlains();

    UFUNCTION(BlueprintCallable, Category = "Terrain Features")
    void CreateCoastalAreas();

protected:
    // Main terrain actor reference
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain", meta = (AllowPrivateAccess = "true"))
    class ALandscape* MainLandscape;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain", meta = (AllowPrivateAccess = "true"))
    class AStaticMeshActor* TerrainPlane;

    // Terrain configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Config")
    float TerrainSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Config")
    float MaxHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Config")
    float MinHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Config")
    int32 HeightmapResolution;

    // Biome zone boundaries (in world coordinates)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zones")
    FVector SwampZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zones")
    FVector ForestZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zones")
    FVector SavannaZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zones")
    FVector DesertZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zones")
    FVector MountainZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zones")
    float BiomeZoneRadius;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableTerrainStreaming;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float StreamingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxTerrainLOD;

    // Status tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
    bool bTerrainCreated;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
    bool bBiomeZonesAllocated;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
    bool bHeightVariationApplied;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
    float CurrentTerrainSize;

private:
    // Internal terrain generation helpers
    void GenerateHeightmapData();
    void ApplyGeologicalFeatures();
    void SetupTerrainMaterials();
    void ConfigureTerrainPhysics();
    void ValidateTerrainSize();

    // Biome zone calculation helpers
    FVector CalculateBiomeZonePosition(EWorld_BiomeType BiomeType);
    bool IsPositionInBiomeZone(const FVector& Position, EWorld_BiomeType BiomeType);
    void CreateBiomeTransitionZones();

    // Performance optimization helpers
    void SetupLODSystem();
    void ConfigureTerrainCulling();
    void OptimizeTerrainCollision();

    // Heightmap data storage
    TArray<TArray<float>> HeightmapData;
    
    // Biome zone masks
    TArray<TArray<uint8>> BiomeZoneMask;
};