#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"
#include "SharedTypes.h"
#include "World_TerrainLODSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainChunk
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector ChunkCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 ChunkSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 CurrentLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    TArray<AActor*> TerrainActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float DistanceToPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bIsActive;

    FWorld_TerrainChunk()
    {
        ChunkCenter = FVector::ZeroVector;
        ChunkSize = 1000;
        CurrentLOD = 0;
        BiomeType = EBiomeType::Forest;
        DistanceToPlayer = 0.0f;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD0Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD1Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD2Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD3Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxVegetationPerChunk;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxRocksPerChunk;

    FWorld_LODSettings()
    {
        LOD0Distance = 500.0f;
        LOD1Distance = 1500.0f;
        LOD2Distance = 3000.0f;
        LOD3Distance = 6000.0f;
        CullDistance = 10000.0f;
        MaxVegetationPerChunk = 100;
        MaxRocksPerChunk = 50;
    }
};

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_TerrainLODSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_TerrainLODSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core LOD Management
    UFUNCTION(BlueprintCallable, Category = "Terrain LOD")
    void InitializeTerrainLOD();

    UFUNCTION(BlueprintCallable, Category = "Terrain LOD")
    void UpdateTerrainLOD(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Terrain LOD")
    void RegisterTerrainChunk(const FWorld_TerrainChunk& Chunk);

    UFUNCTION(BlueprintCallable, Category = "Terrain LOD")
    void UnregisterTerrainChunk(const FVector& ChunkCenter);

    UFUNCTION(BlueprintCallable, Category = "Terrain LOD")
    int32 CalculateLODLevel(float Distance) const;

    // Chunk Management
    UFUNCTION(BlueprintCallable, Category = "Terrain LOD")
    void CreateTerrainChunk(const FVector& Center, EBiomeType BiomeType, int32 Size = 1000);

    UFUNCTION(BlueprintCallable, Category = "Terrain LOD")
    void DestroyTerrainChunk(const FVector& ChunkCenter);

    UFUNCTION(BlueprintCallable, Category = "Terrain LOD")
    TArray<FWorld_TerrainChunk> GetActiveChunks() const;

    UFUNCTION(BlueprintCallable, Category = "Terrain LOD")
    FWorld_TerrainChunk* GetChunkAtLocation(const FVector& Location);

    // Biome Integration
    UFUNCTION(BlueprintCallable, Category = "Terrain LOD")
    void ApplyBiomeLOD(const FWorld_TerrainChunk& Chunk, int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Terrain LOD")
    void PopulateChunkWithBiome(FWorld_TerrainChunk& Chunk);

    UFUNCTION(BlueprintCallable, Category = "Terrain LOD")
    void CullDistantChunks(const FVector& PlayerLocation);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Terrain LOD")
    void GetLODStatistics(int32& ActiveChunks, int32& TotalActors, float& AverageDistance) const;

    UFUNCTION(BlueprintCallable, Category = "Terrain LOD")
    void SetLODSettings(const FWorld_LODSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Terrain LOD")
    FWorld_LODSettings GetLODSettings() const;

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, Category = "Terrain LOD", CallInEditor)
    void DebugDrawChunks();

    UFUNCTION(BlueprintCallable, Category = "Terrain LOD", CallInEditor)
    void ValidateChunkIntegrity();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FWorld_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    TArray<FWorld_TerrainChunk> TerrainChunks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MaxActiveChunks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableDebugVisualization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceBudgetMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorsPerFrame;

private:
    float LastUpdateTime;
    FVector LastPlayerLocation;
    int32 ProcessedChunksThisFrame;

    // Internal Methods
    void ProcessChunkLOD(FWorld_TerrainChunk& Chunk, int32 NewLODLevel);
    void SpawnVegetationForChunk(FWorld_TerrainChunk& Chunk, int32 LODLevel);
    void SpawnRocksForChunk(FWorld_TerrainChunk& Chunk, int32 LODLevel);
    void ClearChunkActors(FWorld_TerrainChunk& Chunk);
    bool IsChunkInFrustum(const FWorld_TerrainChunk& Chunk) const;
    float CalculateChunkPriority(const FWorld_TerrainChunk& Chunk, const FVector& PlayerLocation) const;
};