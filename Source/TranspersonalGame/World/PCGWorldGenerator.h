// Copyright Transpersonal Game Studio. All Rights Reserved.
// PCGWorldGenerator.h - Core PCG World Generation System

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "Components/ActorComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "PCGWorldGenerator.generated.h"

class UPCGComponent;
class UPCGGraph;
class ALandscape;
class ULandscapeComponent;

/**
 * Biome Types for the Prehistoric World
 * Based on Jurassic/Cretaceous period environments
 */
UENUM(BlueprintType)
enum class EPrehistoricBiome : uint8
{
    TropicalRainforest      UMETA(DisplayName = "Tropical Rainforest"),
    ConiferousForest        UMETA(DisplayName = "Coniferous Forest"),
    FernPrairie             UMETA(DisplayName = "Fern Prairie"),
    RiverDelta              UMETA(DisplayName = "River Delta"),
    VolcanicRegion          UMETA(DisplayName = "Volcanic Region"),
    CoastalPlains           UMETA(DisplayName = "Coastal Plains"),
    SwampLands              UMETA(DisplayName = "Swamp Lands"),
    HighlandPlateau         UMETA(DisplayName = "Highland Plateau")
};

/**
 * Terrain Generation Parameters
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FTerrainGenerationParams
{
    GENERATED_BODY()

    // Base terrain settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 HeightmapResolution = 2017;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 ComponentCountX = 16;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 ComponentCountY = 16;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float TerrainScale = 100.0f;

    // Noise parameters for height generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float NoiseScale = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    int32 NoiseOctaves = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float NoisePersistence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
    float NoiseLacunarity = 2.0f;

    // Biome distribution
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TMap<EPrehistoricBiome, float> BiomeWeights;

    FTerrainGenerationParams()
    {
        // Default biome weights for prehistoric world
        BiomeWeights.Add(EPrehistoricBiome::TropicalRainforest, 0.3f);
        BiomeWeights.Add(EPrehistoricBiome::ConiferousForest, 0.2f);
        BiomeWeights.Add(EPrehistoricBiome::FernPrairie, 0.15f);
        BiomeWeights.Add(EPrehistoricBiome::RiverDelta, 0.1f);
        BiomeWeights.Add(EPrehistoricBiome::SwampLands, 0.1f);
        BiomeWeights.Add(EPrehistoricBiome::CoastalPlains, 0.08f);
        BiomeWeights.Add(EPrehistoricBiome::VolcanicRegion, 0.05f);
        BiomeWeights.Add(EPrehistoricBiome::HighlandPlateau, 0.02f);
    }
};

/**
 * PCG World Generator Component
 * Main component for procedural world generation using UE5's PCG Framework
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(PCG), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPCGWorldGenerator : public UActorComponent
{
    GENERATED_BODY()

public:
    UPCGWorldGenerator();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // Core generation functions
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateWorld();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateTerrain();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateRivers();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateVegetation();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ClearGeneration();

    // Biome-specific generation
    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void GenerateTropicalRainforest(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void GenerateConiferousForest(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void GenerateFernPrairie(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void GenerateSwampLands(const FVector& Location, float Radius);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    EPrehistoricBiome GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    float GetHeightAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FVector GetNormalAtLocation(const FVector& WorldLocation) const;

    // Performance and LOD
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetGenerationLOD(int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableRuntimeGeneration(bool bEnable);

protected:
    // Core PCG components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PCG")
    TObjectPtr<UPCGComponent> PCGComponent;

    // PCG Graphs for different generation tasks
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG Graphs")
    TObjectPtr<UPCGGraph> TerrainGenerationGraph;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG Graphs")
    TObjectPtr<UPCGGraph> BiomeGenerationGraph;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG Graphs")
    TObjectPtr<UPCGGraph> VegetationGenerationGraph;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG Graphs")
    TObjectPtr<UPCGGraph> RiverGenerationGraph;

    // Generation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    FTerrainGenerationParams TerrainParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 WorldSeed = 12345;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    FVector WorldSize = FVector(204800.0f, 204800.0f, 25600.0f); // 20km x 20km world

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseHierarchicalGeneration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableRuntimeGeneration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 GenerationRadius = 5000; // 50m generation radius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 CleanupRadius = 7500; // 75m cleanup radius

    // Landscape reference
    UPROPERTY()
    TWeakObjectPtr<ALandscape> GeneratedLandscape;

private:
    // Internal generation functions
    void InitializePCGComponent();
    void SetupPCGGraphs();
    void GenerateHeightmap();
    void ApplyBiomeDistribution();
    void CreateRiverNetwork();
    void PopulateVegetation();

    // Noise generation utilities
    float GeneratePerlinNoise(float X, float Y, int32 Octaves, float Persistence, float Lacunarity) const;
    float GenerateRidgedNoise(float X, float Y) const;
    float GenerateBiomeNoise(float X, float Y, EPrehistoricBiome BiomeType) const;

    // Biome transition utilities
    float CalculateBiomeBlend(const FVector& Location, EPrehistoricBiome BiomeA, EPrehistoricBiome BiomeB) const;
    TArray<EPrehistoricBiome> GetNearbyBiomes(const FVector& Location, float SearchRadius) const;

    // Performance monitoring
    UPROPERTY()
    float LastGenerationTime;

    UPROPERTY()
    int32 GeneratedPointsCount;

public:
    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWorldGenerationComplete, bool, bSuccess);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnWorldGenerationComplete OnWorldGenerationComplete;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBiomeGenerated, EPrehistoricBiome, BiomeType, FVector, Location);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnBiomeGenerated OnBiomeGenerated;
};

/**
 * World Generation Subsystem
 * Manages world generation across the entire game world
 */
UCLASS()
class TRANSPERSONALGAME_API UPCGWorldSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // World management
    UFUNCTION(BlueprintCallable, Category = "World Management")
    void InitializeWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "World Management")
    void GenerateWorldChunk(const FVector& ChunkCenter, int32 ChunkSize);

    UFUNCTION(BlueprintCallable, Category = "World Management")
    void UnloadWorldChunk(const FVector& ChunkCenter);

    // Streaming management
    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void UpdateStreamingAroundPlayer(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void SetStreamingRadius(float NewRadius);

    // Biome queries
    UFUNCTION(BlueprintCallable, Category = "Biome Queries")
    EPrehistoricBiome GetBiomeAtWorldLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Queries")
    TArray<FVector> FindBiomeLocations(EPrehistoricBiome BiomeType, float SearchRadius) const;

protected:
    UPROPERTY()
    TMap<FIntPoint, TObjectPtr<UPCGWorldGenerator>> ActiveChunks;

    UPROPERTY()
    float StreamingRadius = 10000.0f; // 100m streaming radius

    UPROPERTY()
    FVector LastPlayerLocation = FVector::ZeroVector;

private:
    void CleanupDistantChunks(const FVector& PlayerLocation);
    FIntPoint WorldLocationToChunkCoord(const FVector& WorldLocation) const;
    FVector ChunkCoordToWorldLocation(const FIntPoint& ChunkCoord) const;
};