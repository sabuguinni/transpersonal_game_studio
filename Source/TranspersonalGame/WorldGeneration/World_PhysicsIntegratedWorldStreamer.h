#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Landscape/Landscape.h"
#include "WorldPartition/WorldPartition.h"
#include "SharedTypes.h"
#include "World_PhysicsIntegratedWorldStreamer.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_StreamingChunk
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    FVector ChunkLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float ChunkSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bIsLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bHasPhysicsCollision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float PhysicsComplexity;

    FWorld_StreamingChunk()
    {
        ChunkLocation = FVector::ZeroVector;
        ChunkSize = 25600.0f; // 256m chunks
        BiomeType = EBiomeType::Forest;
        bIsLoaded = false;
        bHasPhysicsCollision = false;
        PhysicsComplexity = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_PhysicsStreamingConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float MaxPhysicsDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float CollisionLODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    int32 MaxActivePhysicsChunks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnablePhysicsStreaming;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float PhysicsUpdateInterval;

    FWorld_PhysicsStreamingConfig()
    {
        MaxPhysicsDistance = 5000.0f;
        CollisionLODDistance = 2500.0f;
        MaxActivePhysicsChunks = 9;
        bEnablePhysicsStreaming = true;
        PhysicsUpdateInterval = 0.5f;
    }
};

/**
 * Physics-Integrated World Streaming Manager
 * Manages dynamic loading/unloading of world chunks with physics optimization
 * Integrates with Performance Optimizer for 60fps maintenance
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_PhysicsIntegratedWorldStreamer : public AActor
{
    GENERATED_BODY()

public:
    AWorld_PhysicsIntegratedWorldStreamer();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Streaming Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    FWorld_PhysicsStreamingConfig StreamingConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float StreamingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float UnloadRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    int32 MaxLoadedChunks;

    // Physics Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnablePhysicsOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float PhysicsLODThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    TArray<FWorld_StreamingChunk> LoadedChunks;

    // Performance Monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsObjects;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsMemoryUsage;

public:
    // Streaming Management
    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void UpdateWorldStreaming(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void LoadChunk(const FVector& ChunkLocation, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void UnloadChunk(const FVector& ChunkLocation);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    bool IsChunkLoaded(const FVector& ChunkLocation) const;

    // Physics Integration
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void OptimizePhysicsForChunk(FWorld_StreamingChunk& Chunk);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void UpdatePhysicsLOD(float DistanceFromPlayer);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void EnablePhysicsForChunk(const FVector& ChunkLocation, bool bEnable);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void MonitorStreamingPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetStreamingMemoryUsage() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForTargetFramerate(float TargetFPS);

    // Biome Integration
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EBiomeType GetBiomeForLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void PreloadBiomeAssets(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void UnloadBiomeAssets(EBiomeType BiomeType);

private:
    // Internal Streaming Logic
    void ProcessStreamingQueue();
    void UpdateChunkPhysics();
    void OptimizeMemoryUsage();
    void ValidateStreamingState();

    // Performance Integration
    void IntegrateWithPerformanceOptimizer();
    void AdjustStreamingBasedOnPerformance();

    // Timers
    FTimerHandle StreamingUpdateTimer;
    FTimerHandle PhysicsOptimizationTimer;
    FTimerHandle PerformanceMonitorTimer;

    // Streaming State
    TArray<FVector> PendingLoadQueue;
    TArray<FVector> PendingUnloadQueue;
    float LastStreamingUpdate;
    bool bIsStreamingActive;
};