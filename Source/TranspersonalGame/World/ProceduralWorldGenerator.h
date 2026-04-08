/**
 * @file ProceduralWorldGenerator.h
 * @brief Procedural World Generator for 50km² consciousness-aware terrain
 * @author Transpersonal Game Studio - Procedural World Generator
 * @version 1.0
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Landscape/Classes/Landscape.h"
#include "ProceduralMeshComponent.h"
#include "../Core/ConsciousnessSystem.h"
#include "../Performance/PhysicsOptimizer.h"
#include "ProceduralWorldGenerator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessResonance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EnergyDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<UStaticMesh*> VegetationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInterface* TerrainMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor AuraColor;

    FBiomeData()
    {
        BiomeName = TEXT("Default");
        ConsciousnessResonance = 1.0f;
        EnergyDensity = 0.5f;
        AuraColor = FLinearColor::White;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChunkData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D ChunkCoordinates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasConsciousnessNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<AActor*> SpawnedActors;

    FChunkData()
    {
        ChunkCoordinates = FVector2D::ZeroVector;
        bIsLoaded = false;
        bHasConsciousnessNodes = false;
        ConsciousnessLevel = 0.0f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API AProceduralWorldGenerator : public AActor
{
    GENERATED_BODY()

public:
    AProceduralWorldGenerator();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // World Generation Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 WorldSizeKm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 ChunkSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 LoadRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    float NoiseScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 Seed;

    // Consciousness Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    TObjectPtr<UConsciousnessSystem> ConsciousnessSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float ConsciousnessInfluenceRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    TArray<FBiomeData> ConsciousnessBiomes;

    // Performance Optimization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    TObjectPtr<UPhysicsOptimizer> PhysicsOptimizer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseAsyncGeneration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxChunksPerFrame;

    // Biome System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FBiomeData> AvailableBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float BiomeTransitionDistance;

private:
    // Chunk Management
    TMap<FVector2D, FChunkData> LoadedChunks;
    TArray<FVector2D> ChunksToLoad;
    TArray<FVector2D> ChunksToUnload;
    
    // Generation State
    FVector LastPlayerPosition;
    bool bIsGenerating;
    int32 CurrentGenerationFrame;

    // Consciousness Integration
    TArray<FVector> ConsciousnessNodes;
    TMap<FVector2D, float> ChunkConsciousnessLevels;

public:
    // Core Generation Functions
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void InitializeWorld();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void UpdateWorldAroundPlayer(FVector PlayerPosition);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateChunk(FVector2D ChunkCoord);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void UnloadChunk(FVector2D ChunkCoord);

    // Terrain Generation
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    float GenerateHeightAtPosition(FVector2D Position);

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    FBiomeData GetBiomeAtPosition(FVector2D Position, float ConsciousnessLevel);

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void GenerateTerrainMesh(FVector2D ChunkCoord, TArray<FVector>& Vertices, TArray<int32>& Triangles);

    // Consciousness Integration
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void PlaceConsciousnessNodes(FVector2D ChunkCoord);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    float CalculateConsciousnessInfluence(FVector Position);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void UpdateConsciousnessField(float DeltaTime);

    // Vegetation and Objects
    UFUNCTION(BlueprintCallable, Category = "Objects")
    void SpawnVegetation(FVector2D ChunkCoord, const FBiomeData& Biome);

    UFUNCTION(BlueprintCallable, Category = "Objects")
    void SpawnConsciousnessArtifacts(FVector2D ChunkCoord);

    UFUNCTION(BlueprintCallable, Category = "Objects")
    void SpawnMeditationSpots(FVector2D ChunkCoord);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    FVector2D WorldToChunkCoord(FVector WorldPosition);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    FVector ChunkToWorldPosition(FVector2D ChunkCoord);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool IsChunkInRange(FVector2D ChunkCoord, FVector PlayerPosition);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeChunkLOD(FVector2D ChunkCoord, float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePhysicsOptimization();

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChunkLoaded, FVector2D, ChunkCoord);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChunkUnloaded, FVector2D, ChunkCoord);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnConsciousnessNodeActivated, FVector, NodePosition, float, Intensity);

    UPROPERTY(BlueprintAssignable)
    FOnChunkLoaded OnChunkLoaded;

    UPROPERTY(BlueprintAssignable)
    FOnChunkUnloaded OnChunkUnloaded;

    UPROPERTY(BlueprintAssignable)
    FOnConsciousnessNodeActivated OnConsciousnessNodeActivated;
};