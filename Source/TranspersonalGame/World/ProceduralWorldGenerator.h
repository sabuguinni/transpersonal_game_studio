/**
 * @file ProceduralWorldGenerator.h
 * @brief Core procedural world generation system for transpersonal experiences
 * @author Transpersonal Game Studio - Procedural World Generator
 * @date 2024
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Math/RandomStream.h"
#include "../Consciousness/ConsciousnessTypes.h"
#include "ProceduralWorldGenerator.generated.h"

USTRUCT(BlueprintType)
struct FBiomeParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Temperature = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Elevation = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessResonance = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor AuraColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct FChunkData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D ChunkCoordinate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> TerrainVertices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> TerrainIndices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FBiomeParameters BiomeData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsLoaded = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessInfluence = 0.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UProceduralWorldGenerator : public UActorComponent
{
    GENERATED_BODY()

public:
    UProceduralWorldGenerator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core Generation Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 ChunkSize = 64;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 RenderDistance = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    float NoiseScale = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    float HeightMultiplier = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 Seed = 12345;

    // Consciousness Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float ConsciousnessInfluenceRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float ConsciousnessTerrainModifier = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float BiomeTransitionSpeed = 1.0f;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveChunks = 64;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ChunkUpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAsyncGeneration = true;

    // Core Generation Functions
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateChunk(FVector2D ChunkCoordinate);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void UnloadChunk(FVector2D ChunkCoordinate);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FBiomeParameters CalculateBiome(FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    float GetHeightAtLocation(FVector2D WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void UpdateConsciousnessInfluence(FVector PlayerLocation, EConsciousnessState State, float Intensity);

    // Biome System
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void RegisterBiomeTransition(FVector Location, FBiomeParameters FromBiome, FBiomeParameters ToBiome);

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    TArray<FVector> GetSacredSiteLocations(float Radius);

    // Performance Optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeChunkLOD(FVector2D ChunkCoordinate, float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetGenerationPriority(FVector2D ChunkCoordinate, int32 Priority);

protected:
    // Internal Data
    UPROPERTY()
    TMap<FVector2D, FChunkData> LoadedChunks;

    UPROPERTY()
    TArray<FVector2D> ChunkGenerationQueue;

    UPROPERTY()
    FRandomStream RandomGenerator;

    // Noise Generation
    float GeneratePerlinNoise(float X, float Y, int32 Octaves = 4);
    float GenerateRidgedNoise(float X, float Y);
    float GenerateVoronoiNoise(float X, float Y);

    // Biome Calculation
    FBiomeParameters BlendBiomes(const TArray<FBiomeParameters>& Biomes, const TArray<float>& Weights);
    float CalculateConsciousnessResonance(FVector Location);

    // Chunk Management
    void UpdateChunkPriorities();
    void ProcessGenerationQueue();
    bool ShouldUnloadChunk(FVector2D ChunkCoordinate, FVector PlayerLocation);

    // Async Generation
    void StartAsyncChunkGeneration(FVector2D ChunkCoordinate);
    void OnAsyncGenerationComplete(FVector2D ChunkCoordinate, FChunkData GeneratedData);

private:
    float LastChunkUpdateTime = 0.0f;
    FVector LastPlayerLocation = FVector::ZeroVector;
    bool bIsGenerating = false;

    // Sacred Geometry Integration
    TArray<FVector> CalculateSacredGeometryPoints(FVector Center, float Radius);
    float GetGoldenRatioSpiral(float Angle, float Scale);
    FVector GetFibonacciSpherePoint(int32 Index, int32 Total);
};