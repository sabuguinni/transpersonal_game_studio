#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Landscape/Classes/Landscape.h"
#include "Landscape/Classes/LandscapeProxy.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "World_PerformanceTerrainSystem.generated.h"

/**
 * World_PerformanceTerrainSystem - High-performance terrain generation system
 * Integrates with Agent #4's performance optimizations for 60fps terrain rendering
 * Implements LOD-based terrain streaming and dynamic mesh generation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_PerformanceTerrainSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_PerformanceTerrainSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core terrain components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain System")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain System")
    UStaticMeshComponent* TerrainMeshComponent;

    // Performance settings from Agent #4
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "1", ClampMax = "5"))
    int32 TerrainLODLevels = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "500.0", ClampMax = "10000.0"))
    float StreamingDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "100.0", ClampMax = "2000.0"))
    float ChunkSize = 512.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePerformanceOptimizations = true;

    // Terrain generation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float HeightScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation", meta = (ClampMin = "0.001", ClampMax = "1.0"))
    float NoiseScale = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation", meta = (ClampMin = "1", ClampMax = "10"))
    int32 NoiseOctaves = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Generation", meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float NoisePersistence = 0.5f;

    // Biome integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Integration")
    TArray<World_BiomeType> SupportedBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Integration")
    float BiomeTransitionDistance = 200.0f;

    // Materials for different terrain types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* GrassMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* RockMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* SandMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* SnowMaterial;

    // Performance monitoring
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance Monitoring")
    float CurrentFrameTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance Monitoring")
    int32 ActiveTerrainChunks = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance Monitoring")
    int32 RenderedVertices = 0;

public:
    // Core terrain generation functions
    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateTerrainChunk(FVector ChunkLocation, int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void UpdateTerrainLOD(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void StreamTerrainChunks(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeTerrainPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    // Biome integration functions
    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    void ApplyBiomeToTerrain(World_BiomeType BiomeType, FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    World_BiomeType GetBiomeAtLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    void BlendBiomeTransitions();

    // Terrain modification functions
    UFUNCTION(BlueprintCallable, Category = "Terrain Modification")
    void ModifyTerrainHeight(FVector Location, float Radius, float HeightDelta);

    UFUNCTION(BlueprintCallable, Category = "Terrain Modification")
    void CreateTerrainCrater(FVector Location, float Radius, float Depth);

    UFUNCTION(BlueprintCallable, Category = "Terrain Modification")
    void SmoothTerrainArea(FVector Location, float Radius);

    // Performance optimization functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableLODSystem(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTerrainQuality(int32 QualityLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantTerrain(FVector ViewerLocation);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Utilities")
    float GetTerrainHeightAtLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    FVector GetTerrainNormalAtLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    bool IsLocationOnTerrain(FVector Location);

protected:
    // Internal terrain generation
    void GenerateHeightMap(int32 ChunkX, int32 ChunkY, TArray<float>& HeightData);
    void ApplyNoiseToHeightMap(TArray<float>& HeightData, int32 Size);
    void CreateTerrainMesh(const TArray<float>& HeightData, int32 Size, int32 LODLevel);
    
    // Performance optimization internals
    void UpdateChunkLOD(int32 ChunkIndex, float DistanceToPlayer);
    void CullInvisibleChunks(FVector ViewerLocation, FVector ViewDirection);
    void OptimizeVertexCount(int32 TargetVertexCount);

    // Biome blending internals
    void CalculateBiomeWeights(FVector Location, TArray<float>& BiomeWeights);
    UMaterialInterface* BlendBiomeMaterials(const TArray<float>& BiomeWeights);
    void ApplyBiomeEffectsToChunk(int32 ChunkIndex, World_BiomeType BiomeType);

private:
    // Internal data structures
    TArray<FVector> TerrainChunkLocations;
    TArray<int32> ChunkLODLevels;
    TArray<bool> ChunkVisibility;
    TMap<FIntPoint, UStaticMeshComponent*> TerrainChunks;
    
    // Performance tracking
    float LastPerformanceUpdate = 0.0f;
    float PerformanceUpdateInterval = 1.0f;
    
    // Terrain generation cache
    TMap<FIntPoint, TArray<float>> CachedHeightMaps;
    int32 MaxCachedChunks = 50;
};