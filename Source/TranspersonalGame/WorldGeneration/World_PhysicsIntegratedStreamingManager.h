#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldPartition/WorldPartitionStreamingSource.h"
#include "../SharedTypes.h"
#include "World_PhysicsIntegratedStreamingManager.generated.h"

// Forward declarations
class UWorld_PerformanceIntegration;
class UPerf_PhysicsCoordinatorOptimizer;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_StreamingChunk
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    FVector ChunkLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    FVector ChunkSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    int32 ChunkLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bIsLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bIsPhysicsEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float PhysicsComplexity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    TArray<AActor*> ChunkActors;

    FWorld_StreamingChunk()
    {
        ChunkLocation = FVector::ZeroVector;
        ChunkSize = FVector(1000.0f, 1000.0f, 500.0f);
        ChunkLOD = 0;
        bIsLoaded = false;
        bIsPhysicsEnabled = false;
        PhysicsComplexity = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_StreamingMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 LoadedChunks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ActivePhysicsChunks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float StreamingMemoryUsage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsMemoryUsage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float StreamingTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UnloadingTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsUpdateTime;

    FWorld_StreamingMetrics()
    {
        LoadedChunks = 0;
        ActivePhysicsChunks = 0;
        StreamingMemoryUsage = 0.0f;
        PhysicsMemoryUsage = 0.0f;
        StreamingTime = 0.0f;
        UnloadingTime = 0.0f;
        PhysicsUpdateTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_PhysicsIntegratedStreamingManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_PhysicsIntegratedStreamingManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core streaming management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float StreamingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float UnloadingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    int32 MaxLoadedChunks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float ChunkSize;

    // Physics integration settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration")
    bool bEnablePhysicsOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration")
    float PhysicsStreamingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration")
    int32 MaxPhysicsChunks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Integration")
    float PhysicsLODDistance;

    // Performance targets (integration with Agent #4)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxStreamingTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxPhysicsTime;

    // Streaming data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Streaming Data")
    TArray<FWorld_StreamingChunk> StreamingChunks;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance Data")
    FWorld_StreamingMetrics CurrentMetrics;

    // Player tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Tracking")
    FVector LastPlayerLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Tracking")
    float PlayerMovementSpeed;

public:
    // Core streaming functions
    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void UpdateStreamingAroundPlayer(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void LoadChunk(const FVector& ChunkLocation);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void UnloadChunk(const FVector& ChunkLocation);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    bool IsChunkLoaded(const FVector& ChunkLocation) const;

    // Physics integration functions
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void EnablePhysicsForChunk(const FVector& ChunkLocation);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void DisablePhysicsForChunk(const FVector& ChunkLocation);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void OptimizePhysicsLOD(float FrameTime);

    // Performance monitoring (integration with Agent #4's optimizer)
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FWorld_StreamingMetrics GetStreamingMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RegisterWithPhysicsOptimizer();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    FVector GetChunkLocationFromWorld(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    TArray<FVector> GetChunksInRadius(const FVector& CenterLocation, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void ForceUnloadDistantChunks();

private:
    // Internal streaming management
    void InitializeStreamingSystem();
    void UpdateChunkStates(float DeltaTime);
    void ProcessStreamingQueue();
    void OptimizeMemoryUsage();
    
    // Physics integration internals
    void UpdatePhysicsChunks(const FVector& PlayerLocation);
    void AdjustPhysicsComplexity(FWorld_StreamingChunk& Chunk, float Distance);
    
    // Performance optimization
    void MonitorPerformance(float DeltaTime);
    void AdjustStreamingParameters();
    
    // Chunk management
    FWorld_StreamingChunk* FindChunk(const FVector& ChunkLocation);
    void CreateNewChunk(const FVector& ChunkLocation);
    void RemoveChunk(const FVector& ChunkLocation);
    
    // Integration with other systems
    UWorld_PerformanceIntegration* PerformanceIntegrator;
    UPerf_PhysicsCoordinatorOptimizer* PhysicsOptimizer;
    
    // Internal state
    float LastUpdateTime;
    float AccumulatedFrameTime;
    int32 FrameCounter;
    bool bIsOptimizing;
};