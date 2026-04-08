// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "../Consciousness/ConsciousnessSystem.h"
#include "../Physics/PhysicsOptimizer.h"
#include "ProceduralWorldGenerator.generated.h"

USTRUCT(BlueprintType)
struct FBiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessResonance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<UStaticMesh*> TerrainMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<UStaticMesh*> VegetationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor AmbientColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EnergyFlowIntensity;

    FBiomeData()
    {
        BiomeName = TEXT("DefaultBiome");
        ConsciousnessResonance = 1.0f;
        AmbientColor = FLinearColor::White;
        EnergyFlowIntensity = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FChunkData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D ChunkCoordinates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsGenerated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<AActor*> GeneratedActors;

    FChunkData()
    {
        ChunkCoordinates = FVector2D::ZeroVector;
        bIsLoaded = false;
        bIsGenerated = false;
        ConsciousnessLevel = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AProceduralWorldGenerator : public AActor
{
    GENERATED_BODY()

public:
    AProceduralWorldGenerator();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Generation Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 ChunkSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 ViewDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    float NoiseScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 Seed;

    // Consciousness Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float ConsciousnessInfluenceRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float ConsciousnessGenerationThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    bool bUseConsciousnessBasedGeneration;

    // Biome Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FBiomeData> AvailableBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float BiomeTransitionSmoothness;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxChunksPerFrame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ChunkUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseInstancedMeshes;

    // System References
    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    class AConsciousnessSystem* ConsciousnessSystem;

    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    class APhysicsOptimizer* PhysicsOptimizer;

private:
    // Chunk Management
    TMap<FVector2D, FChunkData> LoadedChunks;
    TArray<FVector2D> ChunksToGenerate;
    TArray<FVector2D> ChunksToUnload;
    
    // Generation State
    float LastUpdateTime;
    int32 ChunksGeneratedThisFrame;
    
    // Noise Generation
    class UFastNoise* NoiseGenerator;
    
    // Instanced Mesh Components for Performance
    TMap<UStaticMesh*, UInstancedStaticMeshComponent*> InstancedMeshComponents;

public:
    // Core Generation Functions
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateChunk(FVector2D ChunkCoord);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void UnloadChunk(FVector2D ChunkCoord);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void UpdateWorldAroundPlayer(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FBiomeData GetBiomeAtLocation(FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    float GetConsciousnessInfluenceAtLocation(FVector WorldLocation);

    // Terrain Generation
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    float GetHeightAtLocation(FVector2D WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void GenerateTerrainMesh(FVector2D ChunkCoord, TArray<FVector>& Vertices, TArray<int32>& Triangles);

    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void PlaceVegetation(FVector2D ChunkCoord, const FBiomeData& Biome);

    // Consciousness Integration
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void UpdateChunkConsciousness(FVector2D ChunkCoord);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void SpawnConsciousnessElements(FVector2D ChunkCoord, float ConsciousnessLevel);

    // Performance Optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeChunkPerformance(FVector2D ChunkCoord);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    UInstancedStaticMeshComponent* GetOrCreateInstancedMeshComponent(UStaticMesh* Mesh);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    FVector2D WorldToChunkCoordinates(FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    FVector ChunkToWorldLocation(FVector2D ChunkCoord);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool IsChunkLoaded(FVector2D ChunkCoord);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void ClearAllChunks();

    // Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnChunkGenerated(FVector2D ChunkCoord);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnChunkUnloaded(FVector2D ChunkCoord);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnConsciousnessLevelChanged(FVector2D ChunkCoord, float NewLevel);

private:
    // Internal Generation Functions
    void InitializeNoiseGenerator();
    void UpdateChunkGeneration(float DeltaTime);
    void ProcessChunkQueue();
    float CalculateConsciousnessAtLocation(FVector WorldLocation);
    FBiomeData BlendBiomes(const TArray<FBiomeData>& Biomes, const TArray<float>& Weights);
    void CreateInstancedMeshForChunk(FVector2D ChunkCoord, UStaticMesh* Mesh, const TArray<FTransform>& Transforms);
};