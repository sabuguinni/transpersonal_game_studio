#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Landscape/Landscape.h"
#include "../Performance/Perf_MilestonePerformanceAnalyzer.h"
#include "../SharedTypes.h"
#include "World_MilestoneWorldStreamer.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_StreamingChunk
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    FVector ChunkLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float ChunkSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bIsLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bIsVisible;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    int32 LODLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float DistanceToPlayer;

    FWorld_StreamingChunk()
    {
        ChunkLocation = FVector::ZeroVector;
        ChunkSize = 1000.0f;
        bIsLoaded = false;
        bIsVisible = false;
        LODLevel = 0;
        DistanceToPlayer = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_StreamingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LoadDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UnloadDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveChunks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bPerformanceOptimization;

    FWorld_StreamingSettings()
    {
        LoadDistance = 2000.0f;
        UnloadDistance = 3000.0f;
        MaxActiveChunks = 9;
        UpdateFrequency = 0.5f;
        bPerformanceOptimization = true;
    }
};

/**
 * World streaming system optimized for Milestone 1 "WALK AROUND" requirements
 * Manages terrain chunks, biome transitions, and performance optimization
 * Integrates with Performance Analyzer for real-time optimization
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_MilestoneWorldStreamer : public AActor
{
    GENERATED_BODY()

public:
    AWorld_MilestoneWorldStreamer();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Streaming Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    FWorld_StreamingSettings StreamingSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    TArray<FWorld_StreamingChunk> StreamingChunks;

    // Performance Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    class UPerf_MilestonePerformanceAnalyzer* PerformanceAnalyzer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceBudgetMs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bAdaptiveQuality;

    // Player Tracking
    UPROPERTY(BlueprintReadOnly, Category = "Player")
    class APawn* PlayerPawn;

    UPROPERTY(BlueprintReadOnly, Category = "Player")
    FVector LastPlayerLocation;

    // Timing
    UPROPERTY(BlueprintReadOnly, Category = "Streaming")
    float TimeSinceLastUpdate;

public:
    // Streaming Management
    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void InitializeStreaming();

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void UpdateStreaming();

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void LoadChunk(int32 ChunkIndex);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void UnloadChunk(int32 ChunkIndex);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void UpdateChunkLOD(int32 ChunkIndex, int32 NewLODLevel);

    // Performance Integration
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void IntegrateWithPerformanceAnalyzer(class UPerf_MilestonePerformanceAnalyzer* Analyzer);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsWithinPerformanceBudget() const;

    // Player Tracking
    UFUNCTION(BlueprintCallable, Category = "Player")
    void UpdatePlayerLocation();

    UFUNCTION(BlueprintCallable, Category = "Player")
    float GetDistanceToChunk(int32 ChunkIndex) const;

    // Chunk Management
    UFUNCTION(BlueprintCallable, Category = "Streaming")
    int32 GetNearestChunkIndex() const;

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    TArray<int32> GetChunksInRange(float Range) const;

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void CreateChunkGrid(int32 GridSize, float ChunkSize);

    // Debug & Validation
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ValidateStreamingSystem();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugDrawChunks();

    UFUNCTION(BlueprintPure, Category = "Debug")
    FString GetStreamingStatus() const;

private:
    // Internal helpers
    void UpdateChunkDistances();
    void ProcessChunkLoading();
    void ProcessChunkUnloading();
    void AdaptQualityForPerformance();
    bool ShouldLoadChunk(int32 ChunkIndex) const;
    bool ShouldUnloadChunk(int32 ChunkIndex) const;
};