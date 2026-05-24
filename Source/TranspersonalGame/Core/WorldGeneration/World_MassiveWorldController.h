#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "World_MassiveWorldController.generated.h"

class UWorld_MassiveBiomeManager;
class UWorld_StreamingManager;
class UWorld_MassiveTerrainManager;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WorldChunkData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Data")
    FVector ChunkLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Data")
    int32 ChunkSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Data")
    EWorld_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Data")
    bool bIsLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Data")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Data")
    float LoadPriority;

    FWorld_WorldChunkData()
    {
        ChunkLocation = FVector::ZeroVector;
        ChunkSize = 10000;
        BiomeType = EWorld_BiomeType::Temperate;
        bIsLoaded = false;
        bIsActive = false;
        LoadPriority = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_StreamingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float LoadRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float UnloadRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    int32 MaxActiveChunks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bEnableAsyncLoading;

    FWorld_StreamingSettings()
    {
        LoadRadius = 50000.0f;
        UnloadRadius = 75000.0f;
        MaxActiveChunks = 25;
        UpdateFrequency = 0.5f;
        bEnableAsyncLoading = true;
    }
};

/**
 * Master controller for massive world generation and streaming
 * Coordinates all world generation subsystems for seamless large-scale environments
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_MassiveWorldController : public AActor
{
    GENERATED_BODY()

public:
    AWorld_MassiveWorldController();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

public:
    // Core streaming settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive World")
    FWorld_StreamingSettings StreamingSettings;

    // World size configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive World", meta = (ClampMin = "100000", ClampMax = "10000000"))
    float WorldSize;

    // Chunk management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Massive World", meta = (ClampMin = "1000", ClampMax = "50000"))
    int32 ChunkSize;

    // Performance monitoring
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 ActiveChunksCount;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 LoadedChunksCount;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float StreamingPerformanceScore;

    // System references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
    UWorld_MassiveBiomeManager* BiomeManager;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
    UWorld_StreamingManager* StreamingManager;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Systems")
    UWorld_MassiveTerrainManager* TerrainManager;

    // Core functionality
    UFUNCTION(BlueprintCallable, Category = "Massive World")
    void InitializeMassiveWorld();

    UFUNCTION(BlueprintCallable, Category = "Massive World")
    void UpdateWorldStreaming(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Massive World")
    void LoadChunkAtLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Massive World")
    void UnloadChunkAtLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Massive World")
    FWorld_WorldChunkData GetChunkDataAtLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Massive World")
    TArray<FWorld_WorldChunkData> GetActiveChunks();

    UFUNCTION(BlueprintCallable, Category = "Massive World")
    void OptimizeStreamingPerformance();

    UFUNCTION(BlueprintCallable, Category = "Massive World")
    void SetStreamingSettings(const FWorld_StreamingSettings& NewSettings);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float CalculateStreamingPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceMetrics();

    // Editor utilities
    UFUNCTION(CallInEditor, Category = "Debug")
    void DebugShowChunkBounds();

    UFUNCTION(CallInEditor, Category = "Debug")
    void DebugClearAllChunks();

    UFUNCTION(CallInEditor, Category = "Debug")
    void DebugGenerateTestWorld();

private:
    // Internal chunk management
    TMap<FIntVector, FWorld_WorldChunkData> LoadedChunks;
    TArray<FIntVector> PendingLoadChunks;
    TArray<FIntVector> PendingUnloadChunks;

    // Performance tracking
    float LastUpdateTime;
    float StreamingUpdateTimer;
    int32 FramesSinceLastUpdate;

    // Internal methods
    FIntVector WorldLocationToChunkCoordinate(FVector WorldLocation);
    FVector ChunkCoordinateToWorldLocation(FIntVector ChunkCoordinate);
    void ProcessPendingChunkOperations();
    void UpdatePerformanceMetrics();
    bool ShouldLoadChunk(FIntVector ChunkCoordinate, FVector PlayerLocation);
    bool ShouldUnloadChunk(FIntVector ChunkCoordinate, FVector PlayerLocation);
};