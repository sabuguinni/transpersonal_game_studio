#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldGenerationManager.generated.h"

UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    Savanna     UMETA(DisplayName = "Savanna"),
    Forest      UMETA(DisplayName = "Forest"),
    Desert      UMETA(DisplayName = "Desert"),
    Wetlands    UMETA(DisplayName = "Wetlands"),
    Mountains   UMETA(DisplayName = "Mountains"),
    Coastline   UMETA(DisplayName = "Coastline")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBiomeParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Density = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Scale = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HeightVariation = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D TemperatureRange = FVector2D(15.0f, 35.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D HumidityRange = FVector2D(0.3f, 0.8f);
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FTerrainGenerationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FIntPoint TerrainSize = FIntPoint(4096, 4096);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float HeightScale = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float NoiseScale = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 Octaves = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Persistence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Lacunarity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    TArray<FBiomeParameters> BiomeSettings;
};

/**
 * World Generation Manager - Core system for procedural world generation
 * Handles terrain generation, biome distribution, and world streaming
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorldGenerationManager : public AActor
{
    GENERATED_BODY()

public:
    AWorldGenerationManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Generation Functions
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void InitializeWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateTerrainChunk(const FIntPoint& ChunkCoordinates);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBiomeDistribution();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    float GetHeightAtLocation(const FVector& WorldLocation) const;

    // PCG Integration
    UFUNCTION(BlueprintCallable, Category = "PCG")
    void SetupPCGGraphs();

    UFUNCTION(BlueprintCallable, Category = "PCG")
    void ExecutePCGGeneration(const FIntPoint& ChunkCoordinates);

    // World Partition Integration
    UFUNCTION(BlueprintCallable, Category = "World Partition")
    void InitializeWorldPartition();

    UFUNCTION(BlueprintCallable, Category = "World Partition")
    void LoadWorldPartitionCell(const FIntPoint& CellCoordinates);

    UFUNCTION(BlueprintCallable, Category = "World Partition")
    void UnloadWorldPartitionCell(const FIntPoint& CellCoordinates);

    // Biome System
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void RegisterBiome(EBiomeType BiomeType, const FBiomeParameters& Parameters);

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FBiomeParameters GetBiomeParameters(EBiomeType BiomeType) const;

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utilities")
    FVector WorldToChunkCoordinates(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    FVector ChunkToWorldCoordinates(const FIntPoint& ChunkCoordinates) const;

protected:
    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UPCGComponent* TerrainPCGComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UPCGComponent* BiomePCGComponent;

    // Generation Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    FTerrainGenerationSettings TerrainSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    float ChunkSize = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    int32 MaxActiveChunks = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    float GenerationRadius = 2000.0f;

    // PCG Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG Assets")
    TSoftObjectPtr<UPCGGraph> TerrainPCGGraph;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG Assets")
    TSoftObjectPtr<UPCGGraph> BiomePCGGraph;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG Assets")
    TSoftObjectPtr<UPCGGraph> VegetationPCGGraph;

    // Runtime Data
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TMap<FIntPoint, bool> GeneratedChunks;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TMap<EBiomeType, FBiomeParameters> BiomeRegistry;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    bool bIsWorldGenerationInitialized = false;

private:
    // Internal generation functions
    void GenerateHeightmapData(const FIntPoint& ChunkCoordinates);
    void ApplyBiomeInfluence(const FIntPoint& ChunkCoordinates, EBiomeType BiomeType);
    void CleanupDistantChunks(const FVector& PlayerLocation);

    // Noise generation
    float GeneratePerlinNoise(float X, float Y, int32 Octaves, float Persistence, float Scale) const;
    float SampleBiomeNoise(const FVector& Location) const;

    // World Partition reference
    UPROPERTY()
    class UWorldPartition* WorldPartitionRef;
};