#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "TerrainGenerator.generated.h"

UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    DenseForest      UMETA(DisplayName = "Dense Forest"),
    OpenPlains       UMETA(DisplayName = "Open Plains"),
    RockyHills       UMETA(DisplayName = "Rocky Hills"),
    RiverValley      UMETA(DisplayName = "River Valley"),
    SwampLands       UMETA(DisplayName = "Swamp Lands"),
    VolcanicRegion   UMETA(DisplayName = "Volcanic Region")
};

USTRUCT(BlueprintType)
struct FBiomeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::DenseForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Density = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HeightVariation = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<UStaticMesh>> VegetationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TSoftObjectPtr<UMaterialInterface> TerrainMaterial;
};

USTRUCT(BlueprintType)
struct FTerrainChunk
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector2D ChunkCoordinates = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 ChunkSize = 2048;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FBiomeSettings BiomeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bIsGenerated = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    TWeakObjectPtr<ALandscape> LandscapeActor;
};

/**
 * Procedural World Generator for prehistoric environments
 * Generates terrains, biomes, rivers, and geographical structures using PCG Framework
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATerrainGenerator : public AActor
{
    GENERATED_BODY()

public:
    ATerrainGenerator();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
    // === TERRAIN GENERATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 WorldSizeKm = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 ChunkSizeMeters = 2048;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    float HeightmapScale = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 RandomSeed = 12345;

    // === BIOME CONFIGURATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FBiomeSettings> BiomeConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float BiomeTransitionDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    bool bEnableBiomeBlending = true;

    // === PCG INTEGRATION ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PCG")
    class UPCGComponent* PCGComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    TSoftObjectPtr<UPCGGraphInterface> TerrainGenerationGraph;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    TSoftObjectPtr<UPCGGraphInterface> VegetationGraph;

    // === WORLD PARTITION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    bool bUseWorldPartition = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Partition")
    int32 PartitionCellSize = 12800; // 128m cells

    // === RUNTIME MANAGEMENT ===
    
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<FTerrainChunk> GeneratedChunks;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    bool bIsGenerating = false;

public:
    // === BLUEPRINT CALLABLE FUNCTIONS ===
    
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateWorld();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateChunkAtLocation(const FVector2D& ChunkCoords);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ClearGeneratedWorld();

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FBiomeSettings GetBiomeSettings(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    float GetTerrainHeightAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    FVector GetTerrainNormalAtLocation(const FVector& WorldLocation) const;

    // === DEBUGGING ===
    
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawBiomes();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawChunkBounds();

private:
    // === INTERNAL GENERATION ===
    
    void InitializePCGComponent();
    void SetupBiomeConfigurations();
    void GenerateTerrainChunk(const FVector2D& ChunkCoords, const FBiomeSettings& BiomeSettings);
    void ApplyBiomeToChunk(FTerrainChunk& Chunk);
    
    // === BIOME LOGIC ===
    
    EBiomeType CalculateBiomeForLocation(const FVector2D& Location) const;
    float CalculateNoiseAtLocation(const FVector2D& Location, float Scale, int32 Octaves = 4) const;
    float CalculateTemperature(const FVector2D& Location) const;
    float CalculateHumidity(const FVector2D& Location) const;
    
    // === UTILITIES ===
    
    FVector2D WorldLocationToChunkCoords(const FVector& WorldLocation) const;
    FVector ChunkCoordsToWorldLocation(const FVector2D& ChunkCoords) const;
    bool IsChunkGenerated(const FVector2D& ChunkCoords) const;
    FTerrainChunk* FindChunk(const FVector2D& ChunkCoords);

private:
    // === CACHED DATA ===
    
    UPROPERTY()
    TMap<FVector2D, int32> ChunkIndexMap;

    UPROPERTY()
    class UMaterialParameterCollection* BiomeParameterCollection;

    // === PERFORMANCE TRACKING ===
    
    UPROPERTY()
    float LastGenerationTime = 0.0f;

    UPROPERTY()
    int32 ChunksGeneratedThisFrame = 0;
};