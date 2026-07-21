#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/LevelStreaming.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "World_MassiveWorldStreaming.generated.h"

/**
 * Massive World Streaming System for 200km2 world
 * Handles dynamic loading/unloading of world chunks based on player position
 * Manages memory usage and performance for large-scale environments
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_StreamingChunk
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    FVector ChunkCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float ChunkSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    EWorld_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bIsLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bIsVisible;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float DistanceToPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    int32 ChunkPriority;

    FWorld_StreamingChunk()
    {
        ChunkCenter = FVector::ZeroVector;
        ChunkSize = 1000000.0f; // 10km chunks
        BiomeType = EWorld_BiomeType::Savanna;
        bIsLoaded = false;
        bIsVisible = false;
        DistanceToPlayer = 0.0f;
        ChunkPriority = 0;
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
    float VisibilityDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    int32 MaxLoadedChunks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bEnableAsyncLoading;

    FWorld_StreamingSettings()
    {
        LoadDistance = 5000000.0f;      // 50km
        UnloadDistance = 8000000.0f;    // 80km
        VisibilityDistance = 2000000.0f; // 20km
        MaxLoadedChunks = 25;
        UpdateFrequency = 2.0f;
        bEnableAsyncLoading = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_MassiveWorldStreaming : public AActor
{
    GENERATED_BODY()

public:
    AWorld_MassiveWorldStreaming();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Streaming")
    FWorld_StreamingSettings StreamingSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Streaming")
    TArray<FWorld_StreamingChunk> WorldChunks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Streaming")
    FVector PlayerPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Streaming")
    float TimeSinceLastUpdate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Streaming")
    int32 CurrentLoadedChunks;

public:
    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    void InitializeWorldChunks();

    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    void UpdatePlayerPosition(const FVector& NewPosition);

    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    void UpdateStreamingState();

    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    void LoadChunk(int32 ChunkIndex);

    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    void UnloadChunk(int32 ChunkIndex);

    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    void SetChunkVisibility(int32 ChunkIndex, bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    TArray<int32> GetNearbyChunks(const FVector& Position, float Radius);

    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    void OptimizeMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    FWorld_StreamingChunk GetChunkAtPosition(const FVector& Position);

    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    void ForceLoadBiome(EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    void PreloadPlayerPath(const TArray<FVector>& PathPoints);

private:
    void CalculateChunkDistances();
    void PrioritizeChunks();
    void ProcessLoadQueue();
    void ProcessUnloadQueue();
    void UpdateChunkVisibility();
    void CleanupUnusedResources();
    
    TArray<int32> LoadQueue;
    TArray<int32> UnloadQueue;
    
    UPROPERTY()
    class APlayerController* PlayerController;
    
    UPROPERTY()
    class APawn* PlayerPawn;
};