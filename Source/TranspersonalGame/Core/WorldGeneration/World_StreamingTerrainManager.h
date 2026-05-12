#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "World_StreamingTerrainManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainChunk
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Chunk")
    FVector ChunkLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Chunk")
    FVector2D ChunkSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Chunk")
    int32 LODLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Chunk")
    bool bIsLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Chunk")
    bool bIsVisible;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Chunk")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Chunk")
    float HeightmapScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Chunk")
    TArray<FVector> LandmarkPositions;

    FWorld_TerrainChunk()
    {
        ChunkLocation = FVector::ZeroVector;
        ChunkSize = FVector2D(2000.0f, 2000.0f);
        LODLevel = 0;
        bIsLoaded = false;
        bIsVisible = false;
        BiomeType = EBiomeType::Temperate;
        HeightmapScale = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_StreamingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float LoadDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float UnloadDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float LODDistance1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float LODDistance2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    int32 MaxLoadedChunks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bEnableAsyncLoading;

    FWorld_StreamingSettings()
    {
        LoadDistance = 5000.0f;
        UnloadDistance = 8000.0f;
        LODDistance1 = 3000.0f;
        LODDistance2 = 6000.0f;
        MaxLoadedChunks = 25;
        UpdateFrequency = 2.0f;
        bEnableAsyncLoading = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_StreamingTerrainManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_StreamingTerrainManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Terrain")
    FWorld_StreamingSettings StreamingSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Terrain")
    TArray<FWorld_TerrainChunk> TerrainChunks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Terrain")
    TMap<FVector2D, ALandscapeProxy*> LoadedChunks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Terrain")
    FVector WorldOrigin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Terrain")
    FVector2D WorldSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Terrain")
    int32 ChunksPerSide;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Terrain")
    float ChunkSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Terrain")
    bool bUseWorldPartition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxChunksPerFrame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceBudgetMs;

private:
    float LastUpdateTime;
    TArray<FVector2D> PendingLoads;
    TArray<FVector2D> PendingUnloads;
    class APawn* PlayerPawn;

public:
    UFUNCTION(BlueprintCallable, Category = "Streaming Terrain")
    void InitializeTerrainGrid();

    UFUNCTION(BlueprintCallable, Category = "Streaming Terrain")
    void UpdateStreamingForPlayer(APawn* InPlayerPawn);

    UFUNCTION(BlueprintCallable, Category = "Streaming Terrain")
    void LoadTerrainChunk(const FVector2D& ChunkCoord);

    UFUNCTION(BlueprintCallable, Category = "Streaming Terrain")
    void UnloadTerrainChunk(const FVector2D& ChunkCoord);

    UFUNCTION(BlueprintCallable, Category = "Streaming Terrain")
    void UpdateChunkLOD(const FVector2D& ChunkCoord, int32 NewLODLevel);

    UFUNCTION(BlueprintCallable, Category = "Streaming Terrain")
    FVector2D WorldLocationToChunkCoord(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Streaming Terrain")
    FVector ChunkCoordToWorldLocation(const FVector2D& ChunkCoord) const;

    UFUNCTION(BlueprintCallable, Category = "Streaming Terrain")
    bool IsChunkLoaded(const FVector2D& ChunkCoord) const;

    UFUNCTION(BlueprintCallable, Category = "Streaming Terrain")
    void SetStreamingSettings(const FWorld_StreamingSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Streaming Terrain")
    TArray<FVector2D> GetVisibleChunks(const FVector& ViewLocation, float ViewDistance) const;

    UFUNCTION(BlueprintCallable, Category = "Streaming Terrain")
    void OptimizeMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor = true)
    void AnalyzeStreamingPerformance();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void DebugDrawChunkGrid();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogStreamingStats();
};